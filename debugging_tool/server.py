from threading import Thread
from flask import Flask, jsonify, send_file
from time import sleep
from dataclasses import dataclass
from datetime import datetime
import serial
import uuid
from collections import deque

app = Flask(__name__)

server_id = uuid.uuid4()
print("Server ID: ", server_id)

@dataclass
class SharedData:
    messages: deque
    last_message_index: int

# Keep this the same as max_messages on the client!
max_message_cache = 200
shared_data = SharedData(deque(maxlen=max_message_cache), -1)



@dataclass
class Message:
    payload: bytes
    received_from: int
    received_at: datetime
    internal_id: int

    def readable_time(self):
        return self.received_at.strftime('%H:%M:%S')

    def priority_bit(self):
        return (self.received_from >> 10) & 0b1

    def sender_type(self):
        return (self.received_from >> 8) & 0b11

    def sender_id(self):
        return (self.received_from >> 0) & 0b1111_1111

    def human_readable_type(self):
        return ['controller', 'puzzle', 'needy', 'RESERVED TYPE'][self.sender_type()]

    def _parse_state_update(self):
        timeleft = self.payload[1] << 0x18 | self.payload[2] << 0x10 | self.payload[3] << 0x08 | self.payload[4]
        strikes = self.payload[5]
        max_strikes = self.payload[6]

        return f'{timeleft/1000:3.2f} {strikes:02}/{max_strikes:02}'

    def parse_message(self):
        sender_type = self.sender_type()
        message_type = self.payload[0]
        try:
            if sender_type == 0b00:  # controller
                if message_type == 0:
                    return "ACK"
                elif message_type == 1:
                    return "HELLO"
                elif message_type == 2:
                    return "START " + self._parse_state_update()
                elif message_type == 3:
                    return "STATE " + self._parse_state_update()
                elif message_type == 4:
                    return "SOLVED " + self._parse_state_update()
                elif message_type == 5:
                    return "TIMEOUT " + self._parse_state_update()
                elif message_type == 6:
                    return "STRIKEOUT " + self._parse_state_update()
            elif sender_type == 0b01:  # puzzle
                if message_type == 0:
                    return "REGISTER"
                elif message_type == 1:
                    return f"STRIKE {self.payload[1]}"
                elif message_type == 2:
                    return f"SOLVED"
        except:
            print("Unexpected error: ", sys.exc_info()[0])
            return "PARSE ERROR"

    def serialize(self):
        return {
            'time': self.readable_time(),
            'parsed': self.parse_message(),
            'pretty_raw_sender_id': f'{self.priority_bit():01b} {self.sender_type():02b} {self.sender_id():08b}',
            'raw_message': f"{self.payload.hex(' ')}",
            'human_readable_type': self.human_readable_type(),
            'sender_id': self.sender_id(),
            'internal_id': self.internal_id
        }


def serial_reader(shared_data):
    with serial.Serial('/dev/ttyACM0', 115200, timeout=10) as ser:
        while True:
            line = ser.readline()
            print(line.decode('ascii'))
            if line.startswith(b"message"):
                line = line.decode('ascii')
                line = line.strip()
                parts = line.split(' ')
                sender = int(parts[1])
                message = bytes(int(p) for p in parts[2:])
                received = Message(message, sender, datetime.now(), len(shared_data.messages))
                shared_data.messages.append(received.serialize())
                shared_data.last_message_index += 1
                print(shared_data.last_message_index)

@app.route('/')
def index():
    return send_file('static/index.html')

@app.route('/<last_received>/api.json')
def api(last_received):
    last_received = int(last_received)
    if last_received < shared_data.last_message_index - len(shared_data.messages):
        return jsonify({"server_id": server_id, "newest_msg": shared_data.last_message_index, "messages": list(shared_data.messages)})
    else:
        return jsonify({"server_id": server_id, "newest_msg": shared_data.last_message_index, "messages": list(shared_data.messages)[len(shared_data.messages) - (shared_data.last_message_index - last_received):]})

if __name__ == '__main__':
    thread = Thread(target=serial_reader, args=(shared_data, ))
    thread.start()
    app.run(debug=False, host='0.0.0.0')
