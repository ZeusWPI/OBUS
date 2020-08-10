#include <mcp2515.h>
#include <assert.h>

#define STATE_INACTIVE 0
#define STATE_HELLO    1
#define STATE_GAME     2

#define OBUS_CONTROLLER_ID 0x000

#define OBUS_TYPE_CONTROLLER 0
#define OBUS_TYPE_PUZZLE     1
#define OBUS_TYPE_NEEDY      2

#define OBUS_MSG_LENGTH 8  // Max 8 to fit in a CAN message

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

MCP2515 mcp2515(10);

uint8_t state = STATE_INACTIVE;

void setup() {
	Serial.begin(9600);
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();
	Serial.println("begin");
}

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

void start_hello() {
	state = STATE_HELLO;

  struct can_frame send_frame;
  
	send_frame.can_id = make_id(OBUS_CONTROLLER_ID, false, OBUS_TYPE_CONTROLLER);
	send_frame.can_dlc = OBUS_MSG_LENGTH;

  send_frame.data[0] = OBUS_MSGTYPE_C_HELLO;
 
	mcp2515.sendMessage(&send_frame);
	Serial.println("sent");
}

void receive_hello() {
  struct can_frame receive_frame;
  if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
    Serial.println("Received message");
    if (receive_frame.data[0] ==  OBUS_MSGTYPE_M_HELLO) {
      Serial.println("Hello"); 
    } else {
      Serial.println("Not implemented");
    }
  }
}

void loop() {
  if (state == STATE_INACTIVE) {
    start_hello();  
  } else if (state == STATE_HELLO) {
      receive_hello();
  } else if (state == STATE_GAME) {
    // Game loop  
  }
}
