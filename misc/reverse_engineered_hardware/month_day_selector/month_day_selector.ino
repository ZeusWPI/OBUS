// yellow
#define CLOCK_PIN  6

// orange
#define DATA_PIN 7

// green
#define READ_PIN 5

uint8_t shiftInFixed(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {

 digitalWrite(clockPin, LOW);
 uint8_t value = 0;
 uint8_t i;
 for (i = 0; i < 8; ++i) {
	 if (bitOrder == LSBFIRST)
	 	value |= digitalRead(dataPin) << i;
	 else
	 	value |= digitalRead(dataPin) << (7 - i);
	 digitalWrite(clockPin, HIGH);
	 delayMicroseconds(3);
	 digitalWrite(clockPin, LOW);
 }
 return value;
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

void setup()
{
	pinMode(CLOCK_PIN, OUTPUT);
	pinMode(DATA_PIN, INPUT);
	pinMode(READ_PIN, OUTPUT);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  Serial.println("Goodnight moon!");
}

void loop() // run over and over
{
	// read data in
	digitalWrite(READ_PIN, HIGH);
	delayMicroseconds(3);
	digitalWrite(READ_PIN, LOW);

	// shift data bit by bit
	uint8_t data[4];

	for (int i = 0; i < 4; i++) {
		data[i] =  shiftInFixed(DATA_PIN, CLOCK_PIN, LSBFIRST);
	}
	digitalWrite(CLOCK_PIN, LOW);


	// First byte is select button:
	// 00000001 STOP
	// 00000010 1S
	// 00000100 10S
	// ...
	// 10000000 REAL OUT
	// 00000000 NORM OUT

	// second, third and fourth byte are rotating digits
	// every dial is a nibble, but the nibbles switched place
	// so 0010 1001 (2 9) is in real life 92

	printBits(data[0]);
	for (int i = 1; i < 4; i++) {
		Serial.print(" ");
		Serial.print(data[i] & 0b1111, DEC);
		Serial.print(data[i] >> 4, DEC);
	}
	Serial.println("");
	delay(500);

}
