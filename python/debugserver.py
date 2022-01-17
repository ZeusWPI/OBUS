from threading import Thread
from flask import Flask, jsonify, send_file
from time import sleep
from dataclasses import dataclass
from datetime import datetime
import serial
import uuid
from collections import deque
import sys
from obus import Message

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


def serial_reader(shared_data):
    with serial.Serial('/dev/ttyUSB0', 115200, timeout=0.05) as ser:
        while True:
            line = ser.read(12)
            if not line:
                continue
            print(line)
            if len(line) == 12:
                if line == b'BEGIN START\n' or line[0] > 0b111:
                    continue
                sender = (int(line[0]) << 8) + int(line[1])
                size = int(line[2])
                message = line[3:3+size]
                received = Message(message, sender, datetime.now())
                shared_data.messages.append(received.serialize())
                shared_data.last_message_index += 1
                print(shared_data.last_message_index)

@app.route('/')
def index():
    return send_file('static/debugger.html')

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
