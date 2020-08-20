#include <mcp2515.h>
#include <assert.h>

#include "obus_can.hpp"


MCP2515 mcp2515(10);
bool obuscan_is_init = false;


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


void obuscan_init() {
	obuscan_is_init = true;
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();
}


void obuscan_send(struct obus_message *msg) {
	if (!obuscan_is_init) {
		Serial.println(F("Call obuscan_init first"));
		return;
	}

	struct can_frame send_frame;

	memset(&send_frame.data, 0, CAN_MAX_DLEN);

	uint8_t length = 1;
	send_frame.data[0] = msg->msg_type;

	switch (msg->payload_type) {
		case OBUS_PAYLDTYPE_EMPTY: break;
		case OBUS_PAYLDTYPE_GAMESTATUS:
			send_frame.data[1] = (uint8_t) ((msg->gamestatus.time_left & 0xFF000000) >> 0x18);
			send_frame.data[2] = (uint8_t) ((msg->gamestatus.time_left & 0x00FF0000) >> 0x10);
			send_frame.data[3] = (uint8_t) ((msg->gamestatus.time_left & 0x0000FF00) >> 0x08);
			send_frame.data[4] = (uint8_t) (msg->gamestatus.time_left & 0x000000FF);
			send_frame.data[5] = msg->gamestatus.strikes;
			send_frame.data[6] = msg->gamestatus.max_strikes;
      length = 7;
	}

	send_frame.can_id = _encode_can_id(msg->from, msg->priority);
	send_frame.can_dlc = length;

	mcp2515.sendMessage(&send_frame);
}


bool obuscan_receive(struct obus_message *msg) {
	if (!obuscan_is_init) {
		Serial.println(F("Call obuscan_init first"));
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
	uint8_t payload_type = -1;

	_decode_can_id(receive_frame.can_id, &msg->from, &msg->priority);
	// Controller messages
	// TODO ifdef, ignore not for us and assume for us
	if (msg->from.type == OBUS_TYPE_CONTROLLER) {
		switch (msg_type) {
			case OBUS_MSGTYPE_C_ACK: // fall-through
			case OBUS_MSGTYPE_C_HELLO:
				payload_type = OBUS_PAYLDTYPE_EMPTY;
				break;

			case OBUS_MSGTYPE_C_GAMESTART: // fall-through
			case OBUS_MSGTYPE_C_STATE:
			case OBUS_MSGTYPE_C_SOLVED:
			case OBUS_MSGTYPE_C_TIMEOUT:
			case OBUS_MSGTYPE_C_STRIKEOUT:
				payload_type = OBUS_PAYLDTYPE_GAMESTATUS;
				break;

			default:
				return false;
				break;
		}

	// Module messages
	} else {
		switch (msg_type) {
			case OBUS_MSGTYPE_M_STRIKE:
				payload_type = OBUS_PAYLDTYPE_IDEMPOTENCY_ID;
				break;

			case OBUS_MSGTYPE_M_HELLO:
			case OBUS_MSGTYPE_M_SOLVED:
				payload_type = OBUS_PAYLDTYPE_EMPTY;
				break;

			default:
				return false;
				break;
		}
	}

	switch (payload_type) {
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

		case OBUS_PAYLDTYPE_IDEMPOTENCY_ID:
			msg->idempotency.id = receive_frame.data[1];
			break;

		default:
			assert(false);
			break;
	}

	msg->msg_type = msg_type;
	msg->payload_type = payload_type;

	return true;
}


inline struct obus_message _obuscan_msg(
		struct module from, bool priority, uint8_t msg_type, uint8_t payload_type) {

	struct obus_message msg;
	msg.from = from;
	msg.priority = priority;
	msg.msg_type = msg_type;
	msg.payload_type = payload_type;
	return msg;
}


struct obus_message obuscan_msg_c_payld_gamestatus(
		struct module from, bool priority, uint8_t msg_type,
		uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	assert(from.type == OBUS_TYPE_CONTROLLER);

	struct obus_message msg = _obuscan_msg(
			from, priority, msg_type, OBUS_PAYLDTYPE_GAMESTATUS);
	msg.gamestatus.time_left = time_left;
	msg.gamestatus.strikes = strikes;
	msg.gamestatus.max_strikes = max_strikes;
	return msg;
}


struct obus_message obuscan_msg_c_ack(struct module from) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	return _obuscan_msg(from, false, OBUS_MSGTYPE_C_ACK, OBUS_PAYLDTYPE_EMPTY);
}

struct obus_message obuscan_msg_c_hello(struct module from) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	return _obuscan_msg(from, false, OBUS_MSGTYPE_C_HELLO, OBUS_PAYLDTYPE_EMPTY);
}


struct obus_message obuscan_msg_c_gamestart(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	return obuscan_msg_c_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_GAMESTART, time_left, strikes, max_strikes);
}

struct obus_message obuscan_msg_c_state(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	return obuscan_msg_c_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_STATE, time_left, strikes, max_strikes);
}

struct obus_message obuscan_msg_c_solved(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	return obuscan_msg_c_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_SOLVED, time_left, strikes, max_strikes);
}

struct obus_message obuscan_msg_c_timeout(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	return obuscan_msg_c_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_TIMEOUT, time_left, strikes, max_strikes);
}

struct obus_message obuscan_msg_c_strikeout(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	return obuscan_msg_c_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_STRIKEOUT, time_left, strikes, max_strikes);
}


struct obus_message obuscan_msg_m_hello(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	return _obuscan_msg(from, false, OBUS_MSGTYPE_M_HELLO, OBUS_PAYLDTYPE_EMPTY);
}

struct obus_message obuscan_msg_m_strike(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	return _obuscan_msg(from, false, OBUS_MSGTYPE_M_STRIKE, OBUS_PAYLDTYPE_EMPTY);
}

struct obus_message obuscan_msg_m_solved(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	return _obuscan_msg(from, false, OBUS_MSGTYPE_M_STRIKE, OBUS_PAYLDTYPE_EMPTY);
}
