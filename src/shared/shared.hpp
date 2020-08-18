#ifndef OBUS_SHARED_H
#define OBUS_SHARED_H

#define OBUS_CONTROLLER_ID 0x000

#define OBUS_TYPE_CONTROLLER 0
#define OBUS_TYPE_PUZZLE     1
#define OBUS_TYPE_NEEDY      2

#define OBUS_MSG_LENGTH 8  // Max 8 to fit in a CAN message

#define OBUS_MAX_MODULES      16
#define OBUS_DISC_DURATION     5  // Duration of discovery round in seconds
#define OBUS_UPDATE_INTERVAL 500 // Number of milliseconds between game updates

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

#define CAN_DOMINANT  0
#define CAN_RECESSIVE 1

struct module {
	uint8_t id;
	uint8_t type;
};


uint16_t make_id(uint8_t id, bool priority, uint8_t type) {
	assert(type <= 0x11);

	/* b bb bbbbbbbb
	 * ↓ type  module ID
	 * priority bit
	 */
	return \
		((uint16_t) (priority ? CAN_DOMINANT : CAN_RECESSIVE) << 10) | \
		((uint16_t) type << 8) | \
		(uint16_t) id;
}

#endif /* end of include guard: OBUS_DEFS_H */
