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

#define OBUS_MAX_MODULES 16
#define OBUS_DISC_DURATION 1 // Duration of discovery round in seconds

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
uint8_t connected_modules_ids[OBUS_MAX_MODULES];
uint8_t nr_connected_modules;

unsigned long hello_round_start;

void setup() {
	Serial.begin(9600);
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();

  nr_connected_modules = 0;
  
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

void send_message(unsigned char* message) {
  struct can_frame send_frame;
  
  send_frame.can_id = make_id(OBUS_CONTROLLER_ID, false, OBUS_TYPE_CONTROLLER);
  send_frame.can_dlc = OBUS_MSG_LENGTH;

  memcpy(send_frame.data, message, OBUS_MSG_LENGTH);
 
  mcp2515.sendMessage(&send_frame);
}

void start_hello() {
	state = STATE_HELLO;
  hello_round_start = millis();

  unsigned char message[OBUS_MSG_LENGTH];
  message[0] = OBUS_MSGTYPE_C_HELLO;

  send_message(message);
  
	Serial.println("sent");
}

void send_ack() {
  unsigned char message[OBUS_MSG_LENGTH];
  message[0] = OBUS_MSGTYPE_C_ACK;

  send_message(message);

  Serial.println("Send ACK");
}

void receive_hello() {
  struct can_frame receive_frame;
  unsigned long current_time = millis();
  
  if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
    Serial.println("Received message");
    if (receive_frame.data[0] ==  OBUS_MSGTYPE_M_HELLO) {
      uint8_t module_id = receive_frame.can_id;
      Serial.print("Hello from module ");
      Serial.println(module_id);
      connected_modules_ids[nr_connected_modules] = module_id;
      nr_connected_modules++;
      send_ack();
    } else {
      Serial.println("Not implemented");
    }
  } else if (current_time - hello_round_start > OBUS_DISC_DURATION * 1000) {
    state = STATE_GAME;
    Serial.println("Initializing game");
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
