#ifndef OBUS_CAN_H
#define OBUS_CAN_H

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
	uint8_t payload_type;
	union {
		struct payld_empty empty;
		struct payld_gamestatus gamestatus;
		struct payld_idempotency idempotency;
	};
};

void obuscan_init();
void obuscan_send(struct obus_message *msg);
bool obuscan_receive(struct obus_message *msg);

struct obus_message obuscan_msg_c_payld_gamestatus(
		struct module from, bool priority, uint8_t msg_type,
		uint32_t time_left, uint8_t strikes, uint8_t max_strikes);

struct obus_message obuscan_msg_c_ack(struct module from);
struct obus_message obuscan_msg_c_hello(struct module from);
struct obus_message obuscan_msg_c_gamestart(struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes);
struct obus_message obuscan_msg_c_state(struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes);
struct obus_message obuscan_msg_c_solved(struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes);
struct obus_message obuscan_msg_c_timeout(struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes);
struct obus_message obuscan_msg_c_strikeout(struct module from, uint32_t time_left, uint8_t strikes, uint8_t max_strikes);

struct obus_message obuscan_msg_m_hello(struct module from);
struct obus_message obuscan_msg_m_strike(struct module from);
struct obus_message obuscan_msg_m_solved(struct module from);

#endif /* end of include guard: OBUS_CAN_H */
