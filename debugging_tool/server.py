from threading import Thread
from flask import Flask, jsonify, send_file
from time import sleep
from dataclasses import dataclass
from datetime import datetime
import serial

app = Flask(__name__)
shared_message_log = []


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
                return f"STRIKE {self.payload[5]}"
            elif message_type == 2:
                return f"SOLVED"
        else:
            return f"PARSE ERROR {self.received_from:011b} {self.payload.hex(' ')}"

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


def serial_reader(messagelog):
    with serial.Serial('/dev/ttyUSB0', 115200, timeout=10) as ser:
        while True:
            line = ser.readline()
            print(line.decode('ascii'))
            if line.startswith(b"message"):
                line = line.decode('ascii')
                line = line.strip()
                parts = line.split(' ')
                sender = int(parts[1])
                message = bytes(int(p) for p in parts[2:])
                received = Message(message, sender, datetime.now(), len(messagelog))
                messagelog.append(received.serialize()

@app.route('/')
def index():
    return send_file('static/index.html')

@app.route('/api.json')
def api():
    return jsonify([m for m in shared_message_log])


if __name__ == '__main__':
    thread = Thread(target=serial_reader, args=(shared_message_log, ))
    thread.start()
    app.run(debug=True)
