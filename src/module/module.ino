#include <CAN.h>
#include <ezButton.h>

#define HANDLE_MESSAGE(type) case type: handle_##type(); break;

#define PUZZLE_M_HANLO 0
#define PUZZLE_M_STRIKE 1
#define PUZZLE_M_SOLVED 2


enum gamestate {
  PENDING, RUNNING, SOLVED, TIMEOUT, STRIKEOUT
};

enum controller_m_type {
  ACK, HELLO, GAME_START, GAME_STATE, GAME_SOLVED, GAME_TIMEOUT, GAME_STRIKEOUT
};

typedef struct state {
  gamestate game_state;
  uint32_t time;
  uint8_t cur_strikes;
  uint8_t max_strikes;
} state_s;

state_s state = { .game_state = PENDING, .time = (uint32_t) -1 };

uint16_t id = 0b00100000010;

ezButton green_button(5);  // create ezButton object that attach to pin 5;
ezButton red_button(7);  // create ezButton object that attach to pin 7;

/* Send Hanlo to controller */
void hanlo() {
  Serial.println("-> HANLO");
  CAN.beginPacket(id);
  CAN.write(PUZZLE_M_HANLO); /* Message Type */
  CAN.endPacket();
}

/* Send strike to controller */
void strike() {
  Serial.println("-> STRIKE");
  CAN.beginPacket(id);
  CAN.write(PUZZLE_M_STRIKE); /* Message Type */
  CAN.endPacket();
}

/* Send solve to controller */
void solved() {
  Serial.println("-> SOLVED");
  CAN.beginPacket(id);
  CAN.write(PUZZLE_M_SOLVED); /* Message Type */
  CAN.endPacket();
}

/* Handle Ack message from controller */
void handle_ACK() {
  Serial.println("<- ACK");
}

/* Handle Hello message from controller */
void handle_HELLO() {
  Serial.println("<- HELLO");
  hanlo();
}

/* Handle Game start message from controller */
void handle_GAME_START() {
  Serial.println("<- GAME_START");
  state.game_state = RUNNING;
  handle_GAME_STATE();
}

/* Handle State message from controller */
void handle_GAME_STATE() {
  Serial.println("<- GAME_STATE");
  state.time = 
    ((uint32_t) CAN.read() << 24) |
    ((uint32_t) CAN.read() << 16) | 
    ((uint32_t) CAN.read() << 8) | 
    CAN.read();
  state.cur_strikes = CAN.read();
  state.max_strikes = CAN.read();

  Serial.print("TIME: ");
  Serial.println(state.time);
  Serial.print("CUR_STRIKES: ");
  Serial.println(state.cur_strikes);
  Serial.print("MAX_STRIKES: ");
  Serial.println(state.max_strikes);
}

/* Handle Solved message from controller */
void handle_GAME_SOLVED() {
  Serial.println("<- GAME_SOLVED");
  state.game_state = SOLVED;
}

/* Handle Timeout message from controller */
void handle_GAME_TIMEOUT() {
  Serial.println("<- GAME_TIMEOUT");
  state.game_state = TIMEOUT;
}

/* Handle Strikeout message from controller */
void handle_GAME_STRIKEOUT() {
  Serial.println("<- GAME_STRIKEOUT");
  state.game_state = STRIKEOUT;
}

void doCAN() {
  if (CAN.parsePacket()) {
    Serial.println("--- GOT_CAN ---");
    /* only react to messages from the controller */
    if (CAN.filter(0b00000000000, 0b01100000000)) {
      Serial.println("--- HANDLING ---");
      if (CAN.peek() != -1) {
        uint8_t message_type = CAN.read();
        Serial.print("MESSAGE_TYPE: ");
        Serial.println(message_type, HEX);
        switch(message_type) {
          HANDLE_MESSAGE(ACK)
          HANDLE_MESSAGE(HELLO)
          HANDLE_MESSAGE(GAME_START)
          HANDLE_MESSAGE(GAME_STATE)
          HANDLE_MESSAGE(GAME_SOLVED)
          HANDLE_MESSAGE(GAME_TIMEOUT)
          HANDLE_MESSAGE(GAME_STRIKEOUT)
          default:
            Serial.println("--- NO_HANDLER_FOUND ---");
        }
      }
    } else {
      Serial.println("--- IGNORING ---");
    }
  }
}

void setup() {
  /* Init Serial */
  Serial.begin(9600);
  Serial.println("--- INITIATING ---");
  Serial.print("ID: ");
  Serial.println(id, HEX);

  /* Init Can pins */
  // CAN.setPins(cs,irq);

  /* Set SPI Frequency */
  // CAN.setSPIFrequency(frequency);

  if (!CAN.begin(50E3)) {
    Serial.println("--- FAILED_TO_INIT_CAN ---");
    while(1) {};
  }

  red_button.setDebounceTime(50);
  green_button.setDebounceTime(50);

  Serial.println("--- INITIALIZED ---");
}

void loop() {
  red_button.loop();
  green_button.loop();
  
  /* Handle CAN messages */
  doCAN();

  if (green_button.isPressed()) {
    solved();
  }

  if (red_button.isPressed()) {
    strike();
  }
  // TODO
}
