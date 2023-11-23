#include <mcp2515.h>

#include <obus_util.h>
#include "obus_can.h"

// Chip select for the CAN module
#define PIN_MCP_CS 8

namespace obus_can {

MCP2515 mcp2515(PIN_MCP_CS);
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

uint8_t payload_type(uint8_t module_type, uint8_t module_id, uint8_t msg_type) {
	if (module_type == OBUS_TYPE_CONTROLLER && module_id == OBUS_CONTROLLER_ID) {
		switch (msg_type) {
			case OBUS_MSGTYPE_C_ACK:
			  return OBUS_PAYLDTYPE_MODULEADDR;
			case OBUS_MSGTYPE_C_HELLO:
				return OBUS_PAYLDTYPE_EMPTY;

			case OBUS_MSGTYPE_C_GAMESTART:
			case OBUS_MSGTYPE_C_STATE:
			case OBUS_MSGTYPE_C_SOLVED:
			case OBUS_MSGTYPE_C_TIMEOUT:
			case OBUS_MSGTYPE_C_STRIKEOUT:
				return OBUS_PAYLDTYPE_GAMESTATUS;

			case OBUS_MSGTYPE_C_INFOSTART:
				return OBUS_PAYLDTYPE_INFOSTART;

			default:
				return false;
				break;
		}
	} else if (module_type == OBUS_TYPE_INFO) {
		// Info modules can only send 7 bytes of data
		return OBUS_PAYLDTYPE_INFO;
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


bool init() {
	is_init = true;
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);

	// filter mask is 0x300 because we only want modules with type 0 (controller & info modules)
	mcp2515.setFilterMask(MCP2515::MASK0, false, 0x300);
	mcp2515.setFilter(MCP2515::RXF0, false, 0);
	mcp2515.setFilter(MCP2515::RXF1, false, 0);
	mcp2515.setFilterMask(MCP2515::MASK1, false, 0x300);
	mcp2515.setFilter(MCP2515::RXF2, false, 0);
	mcp2515.setFilter(MCP2515::RXF3, false, 0);
	mcp2515.setFilter(MCP2515::RXF4, false, 0);
	mcp2515.setFilter(MCP2515::RXF5, false, 0);

	mcp2515.setLoopbackMode();
	struct can_frame test_msg;
	test_msg.can_id = 0x01;
	test_msg.can_dlc = 1;
	test_msg.data[0] = 0x23;
	mcp2515.sendMessage(&test_msg);
	struct can_frame recv_msg;
	delay(50);
	if (mcp2515.readMessage(&recv_msg) == MCP2515::ERROR_OK && recv_msg.can_id == 0x01 && recv_msg.can_dlc == 1 && recv_msg.data[0] == 0x23) {
		mcp2515.setNormalMode();
		return true;
	} else {
		return false;
	}
}


bool receive(struct message *msg) {
	if (!is_init) {
		Serial.println(F("E Call init first"));
		return false;
	}

	struct can_frame receive_frame;

	memset(&receive_frame.data, 0, CAN_MAX_DLEN);

	MCP2515::ERROR status = mcp2515.readMessage(&receive_frame);
	if (status == MCP2515::ERROR_NOMSG) {
		return false;
	}

	if (status != MCP2515::ERROR_OK) {
		Serial.println(F("W readMessage returned error"));
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
	switch (payload_type(from.type, from.id, msg_type)) {
		case OBUS_PAYLDTYPE_EMPTY:
			break;

		case OBUS_PAYLDTYPE_GAMESTATUS:
			if (receive_frame.can_dlc < 8) {
				Serial.println(F("W Received illegal gamestatus msg: payload <8"));
				return false;
			}
			msg->gamestatus.time_left           = unpack_4b_into_u32(&(receive_frame.data[1]));
			msg->gamestatus.strikes             = receive_frame.data[5];
			msg->gamestatus.max_strikes         = receive_frame.data[6];
			msg->gamestatus.puzzle_modules_left = receive_frame.data[7];
			break;

		case OBUS_PAYLDTYPE_COUNT:
			if (receive_frame.can_dlc < 2) {
				Serial.println(F("W Received illegal count msg: payload <2"));
				return false;
			}
			msg->count = receive_frame.data[1];
			break;

		case OBUS_PAYLDTYPE_INFO:
			{
				uint8_t data_len = receive_frame.can_dlc - 1;
				memcpy(msg->infomessage.data, &(receive_frame.data[1]), data_len);
				msg->infomessage.len = data_len;
			}
			break;
		case OBUS_PAYLDTYPE_MODULEADDR:
			{
				if (receive_frame.can_dlc < 3) {
					Serial.println(F("W Received illegal moduleaddr msg: payload <3"));
					return false;
				}
				msg->payload_address.type = receive_frame.data[1];
				msg->payload_address.id = receive_frame.data[2];
			}
			break;
		case OBUS_PAYLDTYPE_INFOSTART:
			{
				if (receive_frame.can_dlc < 5) {
					Serial.println(F("W Received illegal infostart msg: payload <5"));
					return false;
				}
				msg->infostart.seed = unpack_4b_into_u32(&(receive_frame.data[1]));
			}
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

bool is_error_condition() {
	return mcp2515.getInterrupts() & MCP2515::CANINTF_ERRIF;
}

uint8_t get_error_flags() {
	return mcp2515.getErrorFlags();
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

	uint8_t pyld_type = payload_type(msg->from.type, msg->from.id, msg->msg_type);
	switch (pyld_type) {
		case OBUS_PAYLDTYPE_EMPTY:
			break;

		case OBUS_PAYLDTYPE_GAMESTATUS:
			pack_u32_into_4b(&(send_frame.data[1]), msg->gamestatus.time_left);
			send_frame.data[5] = msg->gamestatus.strikes;
			send_frame.data[6] = msg->gamestatus.max_strikes;
			send_frame.data[7] = msg->gamestatus.puzzle_modules_left;
			length = 8;
			break;

		case OBUS_PAYLDTYPE_COUNT:
			send_frame.data[1] = msg->count;
			length = 2;
			break;

		case OBUS_PAYLDTYPE_INFO:
			memcpy(&(send_frame.data[1]), msg->infomessage.data, msg->infomessage.len);
			length = msg->infomessage.len + 1;
			break;

		case OBUS_PAYLDTYPE_MODULEADDR:
			send_frame.data[1] = msg->payload_address.type;
			send_frame.data[2] = msg->payload_address.id;
			length = 3;
		break;

		case OBUS_PAYLDTYPE_INFOSTART:
			pack_u32_into_4b(&(send_frame.data[1]), msg->gamestatus.time_left);
			length = 5;
		break;

		default:
			Serial.print(F("E Unknown payload type "));
			Serial.println(pyld_type);
			return;
	}

	send_frame.can_id = _encode_can_id(msg->from, msg->priority);
	send_frame.can_dlc = length;

	mcp2515.sendMessage(&send_frame);
}

}
