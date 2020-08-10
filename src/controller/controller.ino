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
#define OBUS_GAME_DURATION 60 // Duration of the game in seconds
#define OBUS_MAX_STRIKEOUTS 3 // Number of strikeouts allowed until game over
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

MCP2515 mcp2515(10);

uint8_t state = STATE_INACTIVE;
struct module connected_modules_ids[OBUS_MAX_MODULES];
uint8_t nr_connected_modules;
uint8_t strikeouts;
uint8_t game_running;

// TIMERS
uint16_t hello_round_start;
uint16_t game_start;
uint16_t last_update;

void setup() {
	Serial.begin(9600);
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();

  game_running = 0;
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

struct module get_module_info(uint16_t can_id) {
  uint8_t module_type = can_id & 0x0300;
  uint8_t module_id = can_id & 0x00FF;

  struct module module_info;
  module_info.type = module_type;
  module_info.id = module_id;
  return module_info;
}

void send_message(uint8_t* message) {
  struct can_frame send_frame;
  
  send_frame.can_id = make_id(OBUS_CONTROLLER_ID, false, OBUS_TYPE_CONTROLLER);
  send_frame.can_dlc = OBUS_MSG_LENGTH;

  memcpy(send_frame.data, message, OBUS_MSG_LENGTH);
 
  mcp2515.sendMessage(&send_frame);
}

void start_hello() {
	state = STATE_HELLO;
  hello_round_start = millis();
  nr_connected_modules = 0;

  uint8_t message[OBUS_MSG_LENGTH];
  message[0] = OBUS_MSGTYPE_C_HELLO;

  send_message(message);
  
	Serial.println("Start of discovery round");
}

void send_ack() {
  uint8_t message[OBUS_MSG_LENGTH];
  message[0] = OBUS_MSGTYPE_C_ACK;

  send_message(message);
}

void receive_hello() {
  struct can_frame receive_frame;
  uint16_t current_time = millis();
  
  if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
    if (receive_frame.data[0] ==  OBUS_MSGTYPE_M_HELLO) {
      struct module new_module = get_module_info(receive_frame.can_id);
      Serial.print("Registered module ");
      Serial.println(new_module.id);
      connected_modules_ids[nr_connected_modules] = new_module;
      nr_connected_modules++;
      send_ack();
    }
  } else if (current_time - hello_round_start > OBUS_DISC_DURATION * 1000) {
    state = STATE_GAME;
    Serial.println("End of discovery round");
  }
}

void initialize_game() {
  strikeouts = 0;

  uint16_t game_duration_millis = (uint16_t) OBUS_GAME_DURATION * 1000;

  uint8_t message[OBUS_MSG_LENGTH];
  message[0] = OBUS_MSGTYPE_C_GAMESTART;
  message[1] = (uint8_t) ((game_duration_millis & 0xFF000000) >> 0x18);
  message[2] = (uint8_t) ((game_duration_millis & 0x00FF0000) >> 0x10);
  message[3] = (uint8_t) ((game_duration_millis & 0x0000FF00) >> 0x08);
  message[4] = (uint8_t) (game_duration_millis & 0x000000FF);
  message[5] = strikeouts;
  message[6] = OBUS_MAX_STRIKEOUTS;

  send_message(message);

  game_running = 1;
  game_start = millis();
  last_update = game_star();

  Serial.println("Game started");
}

void receive_module_update() {
  if (receive_frame.data[0] == OBUS_MSGTYPE_M_STRIKE) {
    // Handle strikeout
  } else if (receive_frame.data[0] == OBUS_MSGTYPE_M_SOLVED) {
    // Handle solved module
  }
}

void game_loop() {
  
  
  uint16_t current_time = millis();
  uint16_t elapsed_time = current_time - last_update;

}

void loop() {
  if (state == STATE_INACTIVE) {
    start_hello();  
  } else if (state == STATE_HELLO) {
    receive_hello();
  } else if (state == STATE_GAME) {
    if (game_running) {
      game_loop();
    } else {
      initialize_game();  
    }
  }
}
