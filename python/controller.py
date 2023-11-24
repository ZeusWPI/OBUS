from threading import Thread
from flask import Flask, jsonify, send_file, redirect, request, render_template
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import serial
import uuid
from collections import deque
import sys
import enum
import struct
import time
import random
import typing

from obus import Message, ModuleAddress


# This is the OBUS controller
# This program consists of two threads:
#  - a Flask webserver, serving static files and an API that's used by the OBUS web interface and the debugger
#  - a thread that's responsible for communicating over the serial connection to the OBUS USB/CAN adapter
# These threads communicate using two global variables:
#  - web_to_serial (instance of the SharedWebToSerial class); only the Flask thread is allowed to write fields here; the serial thread can read them
#  - serial_to_web (instance of the SharedSerialToWeb class); only the serial thread is allowed to write fields here; the Flask thread can read them
# In addition to the controller interface running on / (so http://localhost:8080/), 
#  there is also a debugger showing all CAN messages on /debugger (so http://localhost:8080/debugger)
# The settings are changeable on /settings (http://localhost:8080/settings), this should only be done after a game is done


import logging
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

INFO_ROUND_DURATION = timedelta(seconds=5)
DISCOVER_ROUND_DURATION = timedelta(seconds=5)
GAMESTATE_UPDATE_INTERVAL = timedelta(seconds=0.5)


@enum.unique
class Gamestate(enum.Enum):
    INACTIVE = 0
    INFO = 1
    DISCOVER = 2
    READY = 3
    GAME = 4
    GAMEOVER = 5



@dataclass
class PuzzleState:
    '''State keeping object for puzzle and needy modules'''
    strike_amount: int = 0
    solved: bool = False


@dataclass
class SharedWebToSerial:
    game_duration: timedelta = timedelta(seconds=30)
    max_allowed_strikes: int = 5
    fixed_seed: typing.Optional[int] = None
    blocked_modules: list[ModuleAddress] = field(default_factory=list)
    start_game: bool = False
    restart_game: bool = False
    want_game_stop: bool = False


@dataclass
class SharedSerialToWeb:
    gamestate: Gamestate = Gamestate.INACTIVE
    info_round_start: datetime = None
    discover_round_start: datetime = None
    game_start: datetime = None
    game_stop: datetime = None
    last_state_update: datetime = None
    registered_modules: dict[ModuleAddress, PuzzleState] = field(default_factory=dict)
    game_stop_cause: str = None
    chosen_seed: int = 0

@dataclass
class DebugShared:
    messages: list
    last_message_index: int

debug_shared = DebugShared([], -1)

app = Flask(__name__)

server_id = uuid.uuid4()
print("Server ID: ", server_id)

web_to_serial = SharedWebToSerial()
serial_to_web = SharedSerialToWeb()


def parse_can_line(ser, debug_shared) -> Message:
    if not ser.in_waiting:
        return None
    line = ser.read(12)
    if len(line) == 12:
        if line == b'BEGIN START\n' or line[0] > 0b111:
            return None
        sender = (int(line[0]) << 8) + int(line[1])
        size = int(line[2])
        if size == 255:
            message = line[3:3+8]
            obj = Message(message, sender, datetime.now(), is_error=True)
        else:
            message = line[3:3+size]
            obj = Message(message, sender, datetime.now())
        debug_shared.messages.append(obj)
        return obj
    return None


def send_message(ser, msg, debug_shared) -> None:
    debug_shared.messages.append(msg)
    # we send the payload padded with null-bytes, but these don't actually get sent
    packed = struct.pack('>HB8s', msg.module_address().as_binary(), len(msg.payload), msg.payload)
    ser.write(packed + b'\n')

def calculate_puzzle_modules_left(serial_to_web) -> int:
    return sum(address.is_puzzle() and not state.solved for address, state in serial_to_web.registered_modules.items())

def calculate_strikes(serial_to_web) -> int:
    return sum(state.strike_amount for state in serial_to_web.registered_modules.values())

def serial_controller(serialport, web_to_serial, serial_to_web, debug_shared):
    with serial.Serial(serialport, 115200, timeout=0.05) as ser:
        serial_to_web.gamestate = Gamestate.INACTIVE

        ser.reset_input_buffer()
        # Restart Arduino
        ser.setDTR(False)
        time.sleep(0.022)
        ser.setDTR(True)

        # TODO send message here to get all modules to stop talking and reset
        ser.reset_input_buffer()
        time.sleep(5)
        while True:
            time.sleep(1/1000)
            if serial_to_web.gamestate == Gamestate.INACTIVE:
                # don't include 0 as possible seed value
                seed = web_to_serial.fixed_seed or random.randrange(1, 2**32)
                assert seed != 0
                serial_to_web.chosen_seed = seed
                send_message(ser, Message.create_controller_infostart(seed), debug_shared)
                serial_to_web.gamestate = Gamestate.INFO
                serial_to_web.info_round_start = datetime.now()
                serial_to_web.registered_modules = {}
            elif serial_to_web.gamestate == Gamestate.INFO:
                parse_can_line(ser, debug_shared) # throw away, TODO keep this and display it
                if datetime.now() - serial_to_web.info_round_start > INFO_ROUND_DURATION:
                    serial_to_web.gamestate = Gamestate.DISCOVER
                    serial_to_web.discover_round_start = datetime.now()
                    send_message(ser, Message.create_controller_hello(),debug_shared)
            elif serial_to_web.gamestate == Gamestate.DISCOVER:
                if datetime.now() - serial_to_web.discover_round_start > DISCOVER_ROUND_DURATION:
                    serial_to_web.gamestate = Gamestate.READY
                msg = parse_can_line(ser, debug_shared)
                if msg is None:
                    continue
                puzzle_address = msg.get_puzzle_register()
                if puzzle_address is None:
                    continue
                if puzzle_address in web_to_serial.blocked_modules:
                    # this is a blocked puzzle module, don't ack it
                    continue
                serial_to_web.registered_modules[puzzle_address] = PuzzleState()
                send_message(ser, Message.create_controller_ack(msg.module_address()),debug_shared)
            elif serial_to_web.gamestate == Gamestate.READY:
                if web_to_serial.start_game:
                    web_to_serial.start_game = False
                    serial_to_web.game_start = datetime.now()
                    serial_to_web.last_state_update = datetime.now()
                    serial_to_web.gamestate = Gamestate.GAME
                    send_message(ser, Message.create_controller_gamestart(web_to_serial.game_duration, 0, web_to_serial.max_allowed_strikes, len(serial_to_web.registered_modules)),debug_shared)
            elif serial_to_web.gamestate == Gamestate.GAME:
                # React to puzzle strike / solve
                msg = parse_can_line(ser, debug_shared)
                if msg is None:
                    pass
                elif (strike_details := msg.get_puzzle_strike_details()):
                    strike_address, strike_amount = strike_details
                    serial_to_web.registered_modules[strike_address].strike_amount = strike_amount
                elif (solved_puzzle_address := msg.get_puzzle_solved()):
                    serial_to_web.registered_modules[solved_puzzle_address].solved = True

                # Handle strikeout / timeout / solve
                time_left = web_to_serial.game_duration - (datetime.now() - serial_to_web.game_start)
                puzzle_modules_left = calculate_puzzle_modules_left(serial_to_web)
                total_strikes = calculate_strikes(serial_to_web)
                if time_left.total_seconds() <= 0:
                    # Pass zero timedelta, because time left can't be negative in the CAN protocol
                    # Timeout case is also handled first, so that in other cases we know there's time left
                    send_message(ser, Message.create_controller_timeout(timedelta(), puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left),debug_shared)
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                    serial_to_web.game_stop_cause = "TIMEOUT"
                elif total_strikes > web_to_serial.max_allowed_strikes:
                    send_message(ser, Message.create_controller_strikeout(time_left, puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left),debug_shared)
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                    serial_to_web.game_stop_cause = "STRIKEOUT"
                elif puzzle_modules_left == 0:
                    send_message(ser, Message.create_controller_solved(time_left, puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left),debug_shared)
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                    serial_to_web.game_stop_cause = "VICTORY"
                elif web_to_serial.want_game_stop:
                    web_to_serial.want_game_stop = False
                    send_message(ser, Message.create_controller_strikeout(time_left, puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left), debug_shared)
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                    serial_to_web.game_stop_cause = "STRIKEOUT"
                if serial_to_web.gamestate == Gamestate.GAMEOVER:
                    serial_to_web.game_stop = datetime.now()
                    continue

                if datetime.now() - serial_to_web.last_state_update > GAMESTATE_UPDATE_INTERVAL:
                    serial_to_web.last_state_update = datetime.now()
                    # Send state update with known-good checked values
                    send_message(ser, Message.create_controller_state(time_left, total_strikes, web_to_serial.max_allowed_strikes, puzzle_modules_left),debug_shared)

            elif serial_to_web.gamestate == Gamestate.GAMEOVER:
                if web_to_serial.restart_game:
                    web_to_serial.restart_game = False
                    serial_to_web.gamestate = Gamestate.INACTIVE



@app.route('/status.json')
def status():
    status_dict = {
        'gamestate': serial_to_web.gamestate.name,
        'server_id': server_id,
        'chosen_seed': serial_to_web.chosen_seed
    }
    if serial_to_web.gamestate == Gamestate.GAME:
        # Send the time left to avoid time syncronisation issues between server and client
        # Client can then extrapolate if it wants to
        status_dict['timeleft'] = (web_to_serial.game_duration - (datetime.now() - serial_to_web.game_start)).total_seconds()
    elif serial_to_web.gamestate == Gamestate.GAMEOVER:
        status_dict['timeleft'] = max(0, (web_to_serial.game_duration - (serial_to_web.game_stop - serial_to_web.game_start)).total_seconds())
        status_dict['cause'] = serial_to_web.game_stop_cause
    if serial_to_web.gamestate in (Gamestate.DISCOVER, Gamestate.READY, Gamestate.GAME, Gamestate.GAMEOVER):
        status_dict['puzzles'] = [
            {'address': address.as_binary(), 'solved': state.solved if address.is_puzzle() else None, 'strikes': state.strike_amount}
            for address, state
            in sorted(serial_to_web.registered_modules.items(), key=(lambda kv: kv[0].as_binary()))
        ]
    status_dict['max_allowed_strikes'] = web_to_serial.max_allowed_strikes
    status_dict['game_duration'] = web_to_serial.game_duration.total_seconds()
    return jsonify(status_dict)

@app.route('/start')
def start():
    if serial_to_web.gamestate == Gamestate.READY:
        web_to_serial.start_game = True
        return 'OK'
    return 'Wrong gamestage'

@app.route('/stop')
def stop():
    if serial_to_web.gamestate == Gamestate.GAME:
        web_to_serial.want_game_stop = True
        return 'OK'
    return 'Wrong gamestage'

@app.route('/restart')
def restart():
    if serial_to_web.gamestate == Gamestate.GAMEOVER:
        web_to_serial.restart_game = True
        return 'OK'
    return 'Wrong gamestage'

@app.route('/<last_received>/api.json')
def api(last_received):
    last_received = int(last_received)
    start = min(len(debug_shared.messages), last_received)
    messages = list(m.serialize() for m in debug_shared.messages[start:])
    return jsonify({"server_id": server_id, "newest_msg": len(debug_shared.messages), "messages": messages})


@app.route('/')
def index_static():
    return send_file('static/controller.html')

@app.route('/debugger')
def debugger_static():
    return send_file('static/debugger.html')

@app.route('/settings')
def settings_static():
    return render_template('settings.html',
                           time_seconds=int(web_to_serial.game_duration.total_seconds()),
                           seed=web_to_serial.fixed_seed or 0,
                           strikes=web_to_serial.max_allowed_strikes)

@app.route('/settings/set', methods=["POST"])
def settings():
    web_to_serial.game_duration = timedelta(seconds=int(request.form.get('time')))
    web_to_serial.max_allowed_strikes = int(request.form.get('strikes'))
    # Set fixed_seed to None if it was 0 in the form
    web_to_serial.fixed_seed = int(request.form.get('seed')) or None
    return redirect('/settings')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 controller.py [serial port]")
        sys.exit()
    if sys.argv[1] != 'mock':
        thread = Thread(target=serial_controller, args=(sys.argv[1], web_to_serial, serial_to_web, debug_shared))
        thread.start()
    app.run(debug=True, host='0.0.0.0', port=8080)
