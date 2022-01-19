from dataclasses import dataclass
from datetime import datetime
import struct
import enum
from typing import NamedTuple


@enum.unique
class ControllerMessageType(enum.Enum):
    ACK = 0
    HELLO = 1
    GAMESTART = 2
    STATE = 3
    SOLVED = 4
    TIMEOUT = 5
    STRIKEOUT = 6
    INFOSTART = 7


class ModuleAddress(NamedTuple):
    module_type: int
    identifier: int

    def is_puzzle(self):
        return self.module_type == 1

    def as_binary(self):
        return (self.module_type << 8) | self.identifier


@dataclass
class Message:
    payload: bytes
    received_from: int
    received_at: datetime

    @classmethod
    def create_controller_message(cls, controller_message_type, content):
        return cls(bytes([controller_message_type.value]) + content, 0, datetime.now())

    @classmethod
    def create_controller_infostart(cls, seed):
        assert 0 <= seed <= 0xFFFFFFFF
        return cls.create_controller_message(ControllerMessageType.INFOSTART, struct.pack('>L', seed))

    @classmethod
    def create_controller_ack(cls, module_address):
        return cls.create_controller_message(ControllerMessageType.ACK, struct.pack('>H', module_address.as_binary()))

    @classmethod
    def create_controller_hello(cls):
        return cls.create_controller_message(ControllerMessageType.HELLO, b'')

    @classmethod
    def create_controller_generic_stateupdate(cls, controller_message_type, timeleft, current_strikes, max_strikes, amount_puzzle_modules_left):
        time_left_ms = int(timeleft.total_seconds() * 1000)
        assert 0 <= time_left_ms <= 0xFFFFFFFF
        return cls.create_controller_message(controller_message_type, struct.pack('>LBBB', time_left_ms, current_strikes, max_strikes, amount_puzzle_modules_left))

    @classmethod
    def create_controller_gamestart(cls, *args):
        return cls.create_controller_generic_stateupdate(ControllerMessageType.GAMESTART, *args)

    @classmethod
    def create_controller_state(cls, *args):
        return cls.create_controller_generic_stateupdate(ControllerMessageType.STATE, *args)

    @classmethod
    def create_controller_solved(cls, *args):
        return cls.create_controller_generic_stateupdate(ControllerMessageType.SOLVED, *args)

    @classmethod
    def create_controller_timeout(cls, *args):
        return cls.create_controller_generic_stateupdate(ControllerMessageType.TIMEOUT, *args)

    @classmethod
    def create_controller_strikeout(cls, *args):
        return cls.create_controller_generic_stateupdate(ControllerMessageType.STRIKEOUT, *args)

    def get_puzzle_register(self):
        '''Returns the address of the puzzle if this is a register puzzle message, None otherwise'''
        if self.sender_type() == 1 and self.payload[0] == 0:
            return self.module_address()
        return None

    def get_puzzle_strike_details(self):
        '''Returns the address and amount of strikes of the puzzle if this is a puzzle strike message, None otherwise'''
        if self.sender_type() == 1 and self.payload[0] == 1:
            return (self.module_address(), self.payload[1])
        return None

    def get_puzzle_solved(self):
        '''Returns the address of the puzzle if this is a solved puzzle message, None otherwise'''
        if self.sender_type() == 1 and self.payload[0] == 2:
            return self.module_address()
        return None

    def readable_time(self):
        return self.received_at.strftime('%H:%M:%S')

    def priority_bit(self):
        return (self.received_from >> 10) & 0b1

    def sender_type(self):
        return (self.received_from >> 8) & 0b11

    def sender_id(self):
        return (self.received_from >> 0) & 0b1111_1111

    def module_address(self):
        return ModuleAddress(self.sender_type(), self.sender_id())

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
