#include <mcp2515.h>

MCP2515 mcp2515(10);

void setup() {
  Serial.begin(9600);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_50KBPS);
  mcp2515.setNormalMode();
  Serial.println("begin");
}

void loop() {
  char message[9];
  message[8] = '\0';
  struct can_frame send_frame;
  struct can_frame receive_frame;
  int read = Serial.readBytesUntil('\n', send_frame.data, 8);
  if (read > 0) {
    send_frame.can_id = 0x001;
    send_frame.can_dlc = read;
    mcp2515.sendMessage(&send_frame);
    Serial.println("sent");
  }
  if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
    memcpy(message, receive_frame.data, 8);
    Serial.println(message);
  }
  Serial.println("Waiting for next message\n");
}
