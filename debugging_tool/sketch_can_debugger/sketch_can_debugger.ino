#include <mcp2515.h>

MCP2515 mcp2515(8);

void setup() {
	Serial.begin(115200);
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();
	for (int i = 0; i < 5; i++) {
		Serial.println("begin");
		delay(1000);
	}
}

void loop() {
	struct can_frame receive_frame;
	if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
		Serial.print("message ");
		Serial.print(receive_frame.can_id, DEC);
		for (int i = 0; i < receive_frame.can_dlc; i++) {
			Serial.print(" ");
			Serial.print(receive_frame.data[i], DEC);
		}
		Serial.print("\n");
	}
}
