#include <mcp2515.h>

MCP2515 mcp2515(8);

void setup() {
	Serial.begin(115200);
	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();
	for (int i = 0; i < 5; i++) {
		Serial.print("BEGIN START\n"); // 11 bytes plus newline
		delay(1000);
	}
}

void loop() {
	struct can_frame receive_frame;
	if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
		Serial.write((byte) (receive_frame.can_id >> 8));
		Serial.write((byte) (receive_frame.can_id & 0xff));
		Serial.write((byte) receive_frame.can_dlc);
		for (int i = 0; i < receive_frame.can_dlc; i++) {
			Serial.write((byte) receive_frame.data[i]);
		}
		for (int i = 0; i < (8 - receive_frame.can_dlc); i++) {
			Serial.write('\0');
		}
		Serial.write('\n');
	}
	if (Serial.available()) {
		byte readbuffer[12];
		if ((Serial.readBytes(readbuffer, 12) != 12) || readbuffer[11] != '\n') {
			return;
		}
		struct can_frame send_frame;
		send_frame.can_id = (((uint32_t) readbuffer[0]) << 8) | (readbuffer[1]);
		send_frame.can_dlc = readbuffer[2];
		for (int i = 0; i < 8; i++) {
			send_frame.data[i] = readbuffer[3+i];
		}
		mcp2515.sendMessage(&send_frame);
	}
}
