#include <mcp2515.h>

#include "obus_can.h"

// Chip select for the CAN module
#define MCP_CS 8

namespace obus_can {

MCP2515 mcp2515(MCP_CS);
bool is_init = false;


uint16_t _encode_can_id(struct module mod, bool priority) {
	assert(mod.type <= 0b11);

	/* b bb bbbbbbbb
	 * ↓ type  module ID
	 * priority bit
	 */
	return \
		((uint16_t) (priority ? CAN_DOMINANT : CAN_RECESSIVE) << 10) | \
		((uint16_t) mod.type << 8) | \
		(uint16_t) mod.id;
}

void _decode_can_id(uint16_t can_id, struct module *mod, bool *priority) {
	*priority = ((can_id >> 10) & 1) == CAN_DOMINANT;
	mod->type = (can_id >> 8) & 0b11;
	mod->id = can_id & 0b11111111;

	assert(mod->type <= 0b11);
}

uint8_t payload_type(uint8_t module_type, uint8_t msg_type) {
	if (module_type == OBUS_TYPE_CONTROLLER) {
		switch (msg_type) {
			case OBUS_MSGTYPE_C_ACK:
			case OBUS_MSGTYPE_C_HELLO:
				return OBUS_PAYLDTYPE_EMPTY;

			case OBUS_MSGTYPE_C_GAMESTART:
			case OBUS_MSGTYPE_C_STATE:
			case OBUS_MSGTYPE_C_SOLVED:
			case OBUS_MSGTYPE_C_TIMEOUT:
			case OBUS_MSGTYPE_C_STRIKEOUT:
				return OBUS_PAYLDTYPE_GAMESTATUS;

			default:
				return false;
				break;
		}

	// Module messages
	} else {
		switch (msg_type) {
			case OBUS_MSGTYPE_M_STRIKE:
				return OBUS_PAYLDTYPE_COUNT;

			case OBUS_MSGTYPE_M_HELLO:
			case OBUS_MSGTYPE_M_SOLVED:
				return OBUS_PAYLDTYPE_EMPTY;

			default:
				return -1;
				break;
		}
	}
}


void init() {
	is_init = true;
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();
}


bool receive(struct message *msg) {
	if (!is_init) {
		Serial.println(F("E Call init first"));
		return false;
	}

	struct can_frame receive_frame;

	memset(&receive_frame.data, 0, CAN_MAX_DLEN);

	MCP2515::ERROR status = mcp2515.readMessage(&receive_frame);
	if (status != MCP2515::ERROR_OK) {
		return false;
	}

	// Always at least OBUS message type required
	if (receive_frame.can_dlc < 1) {
		Serial.println(F("W Received illegal msg: payload <1"));
		return false;
	}

	uint8_t msg_type = receive_frame.data[0];

	struct module from;
	bool priority;
	_decode_can_id(receive_frame.can_id, &from, &priority);

	// Controller messages
	switch (payload_type(from.type, msg_type)) {
		case OBUS_PAYLDTYPE_EMPTY:
			break;

		case OBUS_PAYLDTYPE_GAMESTATUS:
			if (receive_frame.can_dlc < 7) {
				Serial.println(F("W Received illegal gamestatus msg: payload <7"));
				return false;
			}
			msg->gamestatus.time_left   =
				((uint32_t) receive_frame.data[1] << 0x18) |
				((uint32_t) receive_frame.data[2] << 0x10) |
				((uint32_t) receive_frame.data[3] << 0x08) |
				((uint32_t) receive_frame.data[4]);
			msg->gamestatus.strikes     = receive_frame.data[5];
			msg->gamestatus.max_strikes = receive_frame.data[6];
			break;

		case OBUS_PAYLDTYPE_COUNT:
			msg->count = receive_frame.data[1];
			break;

		default:
			Serial.println(F("W Couldn't determine payload type"));
			return false;
	}

	msg->from = from;
	msg->priority = priority;
	msg->msg_type = msg_type;

	return true;
}


void send(struct message *msg) {
	if (!is_init) {
		Serial.println(F("E Call init first"));
		return;
	}

	struct can_frame send_frame;

	memset(&send_frame.data, 0, CAN_MAX_DLEN);

	uint8_t length = 1;
	send_frame.data[0] = msg->msg_type;

	switch (payload_type(msg->from.type, msg->msg_type)) {
		case OBUS_PAYLDTYPE_EMPTY:
			break;

		case OBUS_PAYLDTYPE_GAMESTATUS:
			send_frame.data[1] = (uint8_t) ((msg->gamestatus.time_left & 0xFF000000) >> 0x18);
			send_frame.data[2] = (uint8_t) ((msg->gamestatus.time_left & 0x00FF0000) >> 0x10);
			send_frame.data[3] = (uint8_t) ((msg->gamestatus.time_left & 0x0000FF00) >> 0x08);
			send_frame.data[4] = (uint8_t) (msg->gamestatus.time_left & 0x000000FF);
			send_frame.data[5] = msg->gamestatus.strikes;
			send_frame.data[6] = msg->gamestatus.max_strikes;
			length = 7;
			break;

		case OBUS_PAYLDTYPE_COUNT:
			send_frame.data[1] = msg->count;
			length = 2;
			break;

		default:
			Serial.println(F("Unknown payload type"));
			return;
	}

	send_frame.can_id = _encode_can_id(msg->from, msg->priority);
	send_frame.can_dlc = length;

	mcp2515.sendMessage(&send_frame);
}

}
