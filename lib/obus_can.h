#ifndef OBUS_CAN_H
#define OBUS_CAN_H

#include <assert.h>

#define CAN_DOMINANT  0
#define CAN_RECESSIVE 1

#define OBUS_CONTROLLER_ID 0x000

#define OBUS_MSG_LENGTH 8  // Max 8 to fit in a CAN message

#define OBUS_TYPE_CONTROLLER 0
#define OBUS_TYPE_PUZZLE     1
#define OBUS_TYPE_NEEDY      2

#define OBUS_MSGTYPE_C_ACK       0
#define OBUS_MSGTYPE_C_HELLO     1
#define OBUS_MSGTYPE_C_GAMESTART 2
#define OBUS_MSGTYPE_C_STATE     3
#define OBUS_MSGTYPE_C_SOLVED    4
#define OBUS_MSGTYPE_C_TIMEOUT   5
#define OBUS_MSGTYPE_C_STRIKEOUT 6

#define OBUS_MSGTYPE_M_HELLO  0
#define OBUS_MSGTYPE_M_STRIKE 1
#define OBUS_MSGTYPE_M_SOLVED 2

#define OBUS_PAYLDTYPE_EMPTY          0
#define OBUS_PAYLDTYPE_GAMESTATUS     1
#define OBUS_PAYLDTYPE_IDEMPOTENCY_ID 2

struct module {
	uint8_t type;
	uint8_t id;
};

struct payld_empty {};
struct payld_gamestatus {
	uint32_t time_left;
	uint8_t strikes;
	uint8_t max_strikes;
};
struct payld_idempotency {
	uint8_t id;
};


struct obus_message {
	struct module from;
	bool priority;
	uint8_t msg_type;
	union {
		struct payld_empty empty;
		struct payld_gamestatus gamestatus;
		struct payld_idempotency idempotency;
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
uint8_t obuscan_payload_type(uint8_t module_type, uint8_t message_type);


/**
 * Initialize the CAN controller for OBUS messaging
 */
void obuscan_init();
/**
 * Receive a message
 *
 * @param msg Pointer to memory where the received message will be wriitten
 * @return true if a message was received, false otherwise
 */
bool obuscan_receive(struct obus_message *msg);

/**
 * Lowlevel interface to send a message, you may want to use one of the helpers, like
 * obuscan_send_m_strike
 *
 * @param msg Pointer to a message to send
 */
void obuscan_send(struct obus_message *msg);


/**
 * For internal use only
 *
 * Send an OBUS message
 */
inline struct obus_message _obuscan_msg(struct module from, bool priority, uint8_t msg_type) {

	struct obus_message msg;
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
inline void _obuscan_send_payld_gamestatus(
		struct module from, bool priority, uint8_t msg_type,
		uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	struct obus_message msg = _obuscan_msg(from, priority, msg_type);
	msg.gamestatus.time_left = time_left;
	msg.gamestatus.strikes = strikes;
	msg.gamestatus.max_strikes = max_strikes;
	obuscan_send(&msg);
}



/**
 * Send a controller "ACK" OBUS message
 */
inline void obuscan_send_c_ack(struct module from) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	struct obus_message msg = _obuscan_msg(from, false, OBUS_MSGTYPE_C_ACK);
	obuscan_send(&msg);
}

/**
 * Send a controller "hello" OBUS message
 */
inline void obuscan_send_c_hello(struct module from) {
	assert(from.type == OBUS_TYPE_CONTROLLER);
	struct obus_message msg = _obuscan_msg(from, false, OBUS_MSGTYPE_C_HELLO);
	obuscan_send(&msg);
}


/**
 * Send a controller "game start" OBUS message
 */
inline void obuscan_send_c_gamestart(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_obuscan_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_GAMESTART, time_left, strikes, max_strikes);
}

/**
 * Send a controller "state" OBUS message
 */
inline void obuscan_send_c_state(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_obuscan_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_STATE, time_left, strikes, max_strikes);
}

/**
 * Send a controller "solved" OBUS message
 */
inline void obuscan_send_c_solved(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_obuscan_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_SOLVED, time_left, strikes, max_strikes);
}

/**
 * Send a controller "timeout" OBUS message
 */
inline void obuscan_send_c_timeout(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_obuscan_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_TIMEOUT, time_left, strikes, max_strikes);
}

/**
 * Send a controller "strikeout" OBUS message
 */
inline void obuscan_send_c_strikeout(
		struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes) {

	assert(from.type == OBUS_TYPE_CONTROLLER);
	_obuscan_send_payld_gamestatus(
			from, false, OBUS_MSGTYPE_C_STRIKEOUT, time_left, strikes, max_strikes);
}


/**
 * Send a module "hello" OBUS message
 */
inline void obuscan_send_m_hello(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	struct obus_message msg = _obuscan_msg(from, false, OBUS_MSGTYPE_M_HELLO);
	obuscan_send(&msg);
}

/**
 * Send a module "strike" OBUS message
 */
inline void obuscan_send_m_strike(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	struct obus_message msg = _obuscan_msg(from, false, OBUS_MSGTYPE_M_STRIKE);
	obuscan_send(&msg);
}

/**
 * Send a module "solved" OBUS message
 */
inline void obuscan_send_m_solved(struct module from) {
	assert(from.type != OBUS_TYPE_CONTROLLER);
	struct obus_message msg = _obuscan_msg(from, false, OBUS_MSGTYPE_M_STRIKE);
	obuscan_send(&msg);
}

#endif /* end of include guard: OBUS_CAN_H */
