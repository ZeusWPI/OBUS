from dataclasses import dataclass
from datetime import datetime


@dataclass
class Message:
    payload: bytes
    received_from: int
    received_at: datetime

    def readable_time(self):
        return self.received_at.strftime('%H:%M:%S')

    def priority_bit(self):
        return (self.received_from >> 10) & 0b1

    def sender_type(self):
        return (self.received_from >> 8) & 0b11

    def sender_id(self):
        return (self.received_from >> 0) & 0b1111_1111

    @staticmethod
    def human_readable_type(sender_type, sender_id):
        return [('controller' if sender_id == 0 else 'info'), 'puzzle', 'needy', 'RESERVED TYPE'][sender_type]

    def _parse_state_update(self):
        timeleft = self.payload[1] << 0x18 | self.payload[2] << 0x10 | self.payload[3] << 0x08 | self.payload[4]
        strikes = self.payload[5]
        max_strikes = self.payload[6]
        solved_puzzle_modules = self.payload[7]

        return f'{timeleft/1000:3.2f} {strikes:02}/{max_strikes:02} [{solved_puzzle_modules:02}]'

    def parse_message(self):
        sender_type = self.sender_type()
        message_type = self.payload[0]
        try:
            if sender_type == 0b00 and self.sender_id() == 0:  # controller
                if message_type == 0:
                    return f"ACK {Message.human_readable_type(self.payload[1], self.payload[2])} {self.payload[2]}"
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
                elif message_type == 7:
                    return "INFO START"
            elif sender_type == 0b01:  # puzzle
                if message_type == 0:
                    return "REGISTER"
                elif message_type == 1:
                    return f"STRIKE {self.payload[1]}"
                elif message_type == 2:
                    return f"SOLVED"
            elif sender_type == 0b10: # needy
                if message_type == 0:
                    return "REGISTER"
                elif message_type == 1:
                    return f"STRIKE {self.payload[1]}"
            elif sender_type == 0b00 and self.sender_id() != 0: # info
                if message_type == 0:
                    return "FREE INFO MESSAGE"

        except:
            print("Unexpected error: ", sys.exc_info()[0])
        return "PARSE ERROR"

    def serialize(self):
        return {
            'time': self.readable_time(),
            'parsed': self.parse_message(),
            'pretty_raw_sender_id': f'{self.priority_bit():01b} {self.sender_type():02b} {self.sender_id():08b}',
            'raw_message': f"{self.payload.hex(' ')}",
            'human_readable_type': Message.human_readable_type(self.sender_type(), self.sender_id()),
            'sender_id': self.sender_id()
        }
