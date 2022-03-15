from distutils.log import debug
from threading import Thread
from flask import Flask, jsonify, send_file
from time import sleep
from dataclasses import dataclass, field
from datetime import datetime, timedelta
import serial
import uuid
from collections import deque
import sys
import enum
import struct
import time

from obus import Message, ModuleAddress


INFO_ROUND_DURATION = timedelta(seconds=3)
GAMESTATE_UPDATE_INTERVAL = timedelta(seconds=0.5)


@enum.unique
class Gamestate(enum.Enum):
    INACTIVE = 0
    INFO = 1
    DISCOVER = 2
    GAME = 3
    GAMEOVER = 4



@dataclass
class PuzzleState:
    '''State keeping object for puzzle and needy modules'''
    strike_amount: int = 0
    solved: bool = False


@dataclass
class SharedWebToSerial:
    game_duration: timedelta = timedelta(seconds=60)
    max_allowed_strikes: int = 3
    seed: int = 1
    blocked_modules: list[ModuleAddress] = field(default_factory=list)
    start_game: bool = False
    restart_game: bool = False


@dataclass
class SharedSerialToWeb:
    gamestate: Gamestate = Gamestate.INACTIVE
    info_round_start: datetime = None
    discover_round_start: datetime = None
    game_start: datetime = None
    game_stop: datetime = None
    last_state_update: datetime = None
    registered_modules: dict[ModuleAddress, PuzzleState] = field(default_factory=dict)

@dataclass
class DebugShared:
    messages: deque
    last_message_index: int

# Keep this the same as max_messages on the client!
max_message_cache = 200
debug_shared = DebugShared(deque(maxlen=max_message_cache), -1)

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
        message = line[3:3+size]
        obj = Message(message, sender, datetime.now())
        debug_shared.messages.append(obj)
        debug_shared.last_message_index += 1
    return None


def send_message(ser, msg, debug_shared) -> None:
    debug_shared.messages.append(msg)
    debug_shared.last_message_index += 1
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
        # TODO send message here to get all modules to stop talking and reset
        ser.reset_input_buffer()
        time.sleep(5)
        while True:
            if serial_to_web.gamestate == Gamestate.INACTIVE:
                send_message(ser, Message.create_controller_infostart(web_to_serial.seed))
                serial_to_web.gamestate = Gamestate.INFO
                serial_to_web.info_round_start = datetime.now()
                serial_to_web.registered_modules = {}
            elif serial_to_web.gamestate == Gamestate.INFO:
                parse_can_line(ser, debug_shared) # throw away, TODO keep this and display it
                if datetime.now() - serial_to_web.info_round_start > INFO_ROUND_DURATION:
                    serial_to_web.gamestate = Gamestate.DISCOVER
                    send_message(ser, Message.create_controller_hello())
            elif serial_to_web.gamestate == Gamestate.DISCOVER:
                if web_to_serial.start_game:
                    web_to_serial.start_game = False
                    serial_to_web.game_start = datetime.now()
                    serial_to_web.last_state_update = datetime.now()
                    serial_to_web.gamestate = Gamestate.GAME
                    send_message(ser, Message.create_controller_gamestart(web_to_serial.game_duration, 0, web_to_serial.max_allowed_strikes, len(serial_to_web.registered_modules)))
                msg = parse_can_line(ser, debug_shared)
                if msg is None:
                    continue
                puzzle_address = msg.get_puzzle_register()
                if puzzle_address is None:
                    continue
                if puzzle_address in web_to_serial.blocked_modules:
                    # this is blocked puzzle module, don't ack it
                    continue
                serial_to_web.registered_modules[puzzle_address] = PuzzleState()
                send_message(ser, Message.create_controller_ack(msg.module_address()))

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
                    send_message(ser, Message.create_controller_timeout(timedelta(), puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left))
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                elif total_strikes > web_to_serial.max_allowed_strikes:
                    send_message(ser, Message.create_controller_strikeout(time_left, puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left))
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                elif puzzle_modules_left == 0:
                    send_message(ser, Message.create_controller_solved(time_left, puzzle_modules_left, web_to_serial.max_allowed_strikes, puzzle_modules_left))
                    serial_to_web.gamestate = Gamestate.GAMEOVER
                if serial_to_web.gamestate == Gamestate.GAMEOVER:
                    serial_to_web.game_stop = datetime.now()
                    continue

                if datetime.now() - serial_to_web.last_state_update > GAMESTATE_UPDATE_INTERVAL:
                    serial_to_web.last_state_update = datetime.now()
                    # Send state update with known-good checked values
                    send_message(ser, Message.create_controller_state(time_left, total_strikes, web_to_serial.max_allowed_strikes, puzzle_modules_left))

            elif serial_to_web.gamestate == Gamestate.GAMEOVER:
                if web_to_serial.restart_game:
                    web_to_serial.restart_game = False
                    serial_to_web.gamestate = Gamestate.INACTIVE



@app.route('/status.json')
def status():
    status_dict = {
        'gamestate': serial_to_web.gamestate.name,
        'server_id': server_id
    }
    if serial_to_web.gamestate == Gamestate.GAME:
        # Send the time left to avoid time syncronisation issues between server and client
        # Client can then extrapolate if it wants to
        status_dict['timeleft'] = (web_to_serial.game_duration - (datetime.now() - serial_to_web.game_start)).total_seconds()
    elif serial_to_web.gamestate == Gamestate.GAMEOVER:
        status_dict['timeleft'] = max(0, (web_to_serial.game_duration - (serial_to_web.game_stop - serial_to_web.game_start)).total_seconds())

    if serial_to_web.gamestate in (Gamestate.DISCOVER, Gamestate.GAME, Gamestate.GAMEOVER):
        status_dict['puzzles'] = [
            {'address': address.as_binary(), 'solved': state.solved if address.is_puzzle() else None, 'strikes': state.strike_amount}
            for address, state
            in sorted(serial_to_web.registered_modules.items(), key=(lambda kv: kv[0].as_binary()))
        ]
    status_dict['max_allowed_strikes'] = web_to_serial.max_allowed_strikes
    status_dict['game_duration'] = web_to_serial.game_duration.total_seconds()
    print(status_dict)
    return jsonify(status_dict)

@app.route('/start')
def start():
    if serial_to_web.gamestate == Gamestate.DISCOVER:
        web_to_serial.start_game = True
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
    if last_received < debug_shared.last_message_index - len(debug_shared.messages):
        return jsonify({"server_id": server_id, "newest_msg": debug_shared.last_message_index, "messages": list(debug_shared.messages)})
    else:
        return jsonify({"server_id": server_id, "newest_msg": debug_shared.last_message_index, "messages": list(debug_shared.messages)[len(debug_shared.messages) - (debug_shared.last_message_index - last_received):]})


@app.route('/')
def index():
    return send_file('static/controller.html')

@app.route('/debugger')
def debugger():
    return send_file('static/debugger.html')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 controller.py [serial port]")
        sys.exit()
    if sys.argv[1] != 'mock':
        thread = Thread(target=serial_controller, args=(sys.argv[1], web_to_serial, serial_to_web, debug_shared))
        thread.start()
    app.run(debug=False, host='0.0.0.0', port=8080)
