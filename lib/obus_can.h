#ifndef OBUS_CAN_H
#define OBUS_CAN_H

#include "Arduino.h"
#include <assert.h>

#define CAN_DOMINANT  0
#define CAN_RECESSIVE 1

#define OBUS_CONTROLLER_ID 0x000

#define OBUS_MSG_LENGTH 8  // Max 8 to fit in a CAN message

#define OBUS_TYPE_CONTROLLER 0
#define OBUS_TYPE_INFO       0
#define OBUS_TYPE_PUZZLE     1
#define OBUS_TYPE_NEEDY      2

#define OBUS_MSGTYPE_C_ACK       0
#define OBUS_MSGTYPE_C_HELLO     1
#define OBUS_MSGTYPE_C_GAMESTART 2
#define OBUS_MSGTYPE_C_STATE     3
#define OBUS_MSGTYPE_C_SOLVED    4
#define OBUS_MSGTYPE_C_TIMEOUT   5
#define OBUS_MSGTYPE_C_STRIKEOUT 6
#define OBUS_MSGTYPE_C_INFOSTART 7

#define OBUS_MSGTYPE_M_HELLO  0
#define OBUS_MSGTYPE_M_STRIKE 1
#define OBUS_MSGTYPE_M_SOLVED 2

#define OBUS_MSGTYPE_I_INFOMESSAGE 0

#define OBUS_PAYLDTYPE_EMPTY      0
#define OBUS_PAYLDTYPE_GAMESTATUS 1
#define OBUS_PAYLDTYPE_COUNT      2
#define OBUS_PAYLDTYPE_INFO       3
#define OBUS_PAYLDTYPE_MODULEADDR 4
#define OBUS_PAYLDTYPE_INFOSTART  5

#define OBUS_PAYLD_INFO_MAXLEN (OBUS_MSG_LENGTH - 1)

namespace obus_can {

struct module {
	uint8_t type;
	uint8_t id;
};

struct payld_empty {};
struct payld_gamestatus {
	uint32_t time_left;
	uint8_t strikes;
	uint8_t max_strikes;
	uint8_t puzzle_modules_left;
};
struct payld_infomessage {
	uint8_t len;
	uint8_t data[OBUS_PAYLD_INFO_MAXLEN];
};
struct payld_infostart {
	uint32_t seed;
};


struct message {
	struct module from;
	bool priority;
	uint8_t msg_type;
	union {
		struct payld_empty empty;
		struct payld_gamestatus gamestatus;
		uint8_t count;
		struct payld_infomessage infomessage;
		struct module payload_address;
		struct payld_infostart infostart;
	};
};


/**
 * Determine payload type for message
 *
 * @param module_type One of OBUS_TYPE_*
 * @param module_type One of OBUS_MSGTYPE_*
 *
 * @return One of OBUS_PAYLDTYPE_*
 */
uint8_t payload_type(uint8_t module_type, uint8_t message_type);


/**
 * Initialize the CAN controller for OBUS messaging
 *
 * @return true if initialization and self-test succeeded, false otherwise
 */
bool init();
/**
 * Receive a message
 *
 * @param msg Pointer to memory where the received message will be written
 * @return true if a message was received, false otherwise
 */
bool receive(struct message *msg);

/**
 * Lowlevel interface to send a message, you may want to use one of the helpers, like
 * send_m_strike
 *
 * @param msg Pointer to a message to send
 */
void send(struct message *msg);

bool is_error_condition();

uint8_t get_error_flags();

/**
 * For internal use only
 *
 * Send an OBUS message
 */
inline struct message _msg(struct module from, bool priority, uint8_t msg_type) {

	struct message msg;
	msg.from = from;
	msg.priority = priority;
	msg.msg_type = msg_type;
	return msg;
}


/**
 * For internal use only
 *
 * Send a controller OBUS message with a gamestatus payload
 */
inline void _send_payld_gamestatus(
		struct module from, bool priority, uint8_t msg_type,
		uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {

	struct message msg = _msg(from, priority, msg_type);
	msg.gamestatus.time_left = time_left;
	msg.gamestatus.strikes = strikes;
	msg.gamestatus.max_strikes = max_strikes;
	msg.gamestatus.puzzle_modules_left = puzzle_modules_left;
	send(&msg);
}



/**
 * Send a controller "ACK" OBUS message
 */
inline void send_c_ack(struct module from, struct module payload_address) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	struct message msg = _msg(from, false, OBUS_MSGTYPE_C_ACK);
	msg.payload_address = payload_address;
	send(&msg);
}

/**
 * Send a controller "hello" OBUS message
 */
inline void send_c_hello(struct module from) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	struct message msg = _msg(from, false, OBUS_MSGTYPE_C_HELLO);
	send(&msg);
}


/**
 * Send a controller "game start" OBUS message
 */
inline void send_c_gamestart(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_GAMESTART, time_left, strikes, max_strikes, puzzle_modules_left);
}

/**
 * Send a controller "state" OBUS message
 */
inline void send_c_state(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_STATE, time_left, strikes, max_strikes, puzzle_modules_left);
}

/**
 * Send a controller "solved" OBUS message
 */
inline void send_c_solved(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_SOLVED, time_left, strikes, max_strikes, puzzle_modules_left);
}

/**
 * Send a controller "timeout" OBUS message
 */
inline void send_c_timeout(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_TIMEOUT, time_left, strikes, max_strikes, puzzle_modules_left);
}

/**
 * Send a controller "info start" OBUS message
 */
inline void send_c_infostart(struct module from, uint32_t seed) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	struct message msg = _msg(from, false, OBUS_MSGTYPE_C_INFOSTART);
	msg.infostart.seed = seed;
	send(&msg);
}

/**
 * Send a controller "strikeout" OBUS message
 */
inline void send_c_strikeout(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_STRIKEOUT, time_left, strikes, max_strikes, puzzle_modules_left);
}


/**
 * Send a module "hello" OBUS message
 */
inline void send_m_hello(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	struct message msg = _msg(from, false, OBUS_MSGTYPE_M_HELLO);
	send(&msg);
}

/**
 * Send a module "strike" OBUS message
 */
inline void send_m_strike(struct module from, uint8_t count) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	struct message msg = _msg(from, false, OBUS_MSGTYPE_M_STRIKE);
	msg.count = count;
	send(&msg);
}

/**
 * Send a module "solved" OBUS message
 */
inline void send_m_solved(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	struct message msg = _msg(from, false, OBUS_MSGTYPE_M_SOLVED);
	send(&msg);
}

/**
 * Send an info module "information" OBUS message
 */
inline void send_i_infomessage(struct module from, uint8_t *data, uint8_t data_len) {
	assert(from.type == OBUS_TYPE_INFO && from.id != OBUS_CONTROLLER_ID);

	if (data_len > OBUS_PAYLD_INFO_MAXLEN) {
		Serial.println(F("E Info payload too long"));
		return;
	}

	struct message msg = _msg(from, false, OBUS_MSGTYPE_I_INFOMESSAGE);
	msg.infomessage.len = data_len;
	memset(msg.infomessage.data, 0, OBUS_PAYLD_INFO_MAXLEN);
	memcpy(msg.infomessage.data, data, data_len);
	send(&msg);
}

inline bool is_from_controller(struct module from) {
	return from.type == OBUS_TYPE_CONTROLLER && from.id == OBUS_CONTROLLER_ID;
}

}

#endif /* end of include guard: OBUS_CAN_H */
