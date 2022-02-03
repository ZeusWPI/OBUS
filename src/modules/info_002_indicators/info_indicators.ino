// (c) 2022, redfast00
// See the LICENSE file for conditions for copying

/* Format of the info message

length = 5 bytes
id = 2

message:
X X X X A
------v  -> the labeled indicators
      the colored indicators

The first 4 bytes are for the 8 colored indicators, every
byte contains 2 indices into the color table. The first colored indicator index is
in the first (or most sigificant) 4 bits of the first byte, the second colored indicator
index in the last 4 bits of the first byte, and so on.

The color table is defined below in the `colortable` variable

The fifth byte is for the labeled indicators. The most significant bit (128) is
for the WPI label light, the least sigificant bit (1) for the DRV label light. The bit is 1
if the light is on, 0 otherwise. All labels, in order:

WPI  0b10000000
KEL  0b01000000
DER  0b00100000
ZEUS 0b00010000
JDV  0b00001000
QRV  0b00000100
PSE  0b00000010
DRV  0b00000001

Some code to loop over all colored indicator values, assuming the info message was saved in `buffer`

```
for (uint8_t idx = 0; idx < 8; idx++) {
	coloridx = (buffer[idx / 2] >> (4 * (1 - (idx & 1)))) & 0b111;
}
```
*/

uint8_t colortable[8][3] = {
	{20, 0, 0},  // red    0b0000
	{0, 20, 0},  // green  0b0001
	{0, 0, 20},	 // blue   0b0010
	{10, 10, 0}, // yellow 0b0011
	{0, 10, 10}, // cyan   0b0100
	{15, 5, 0},  // orange 0b0101
	{15, 0, 5},  // pink   0b0110
	{5, 0, 15}   // purple 0b0111
};


#include <obus_module.h>
#include <Adafruit_NeoPixel.h>
#define WS2812_PIN 6
#define BUTTON_PIN 9

uint8_t led_pins[8] = {A0, A1, A2, A3, A4, A5, 3, 2};


Adafruit_NeoPixel pixels(8, WS2812_PIN, NEO_GRB + NEO_KHZ800);
uint8_t led_buffer[5] = {0b00000001, 0b00100011, 0b01000101, 0b01100111, 0b11101100};
bool render_now = true;



void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_INFO, 2);
  pixels.begin();
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	for (int i = 0; i < 8; i++) {
		pinMode(led_pins[i], OUTPUT);
		digitalWrite(led_pins[i], true);
	}
}

obus_can::message message;

void render(bool realcolors) {
	render_now = false;
	for (uint8_t idx = 0; idx < 8; idx++) {
		uint8_t coloridx = idx;
		if (realcolors) {
			coloridx = (led_buffer[idx / 2] >> (4 * (1 - (idx & 1)))) & 0b111;
		}
		pixels.setPixelColor(idx, pixels.Color(colortable[coloridx][0], colortable[coloridx][1], colortable[coloridx][2]));
	}
	pixels.show();
	for (int i = 0; i < 8; i++) {
		digitalWrite(led_pins[i], ((led_buffer[4] >> i) & 0b1) && realcolors);
	}
}

void loop() {
	obus_module::loopInfo(&message, info_generator);
	render(digitalRead(BUTTON_PIN) == HIGH);
}

int info_generator(uint8_t* buffer) {
	for (int i = 0; i < 4; i++) {
		led_buffer[i] = random(256) & 0b01110111;
	}
	led_buffer[4] = random(256);
	render_now = true;
	memcpy(buffer, led_buffer, 5);
	return 5;
}
