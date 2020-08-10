#include <mcp2515.h>
#include "pitches.h"

MCP2515 mcp2515(10);

uint16_t id = 0b01100000011;
// type: 3 (other)
// module-id: 3

int soundpin = 9;


void setup() {
  Serial.begin(9600);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_50KBPS);
  mcp2515.setNormalMode();
  Serial.println("begin");

  // play_sound();
}

void loop() {
  char message[9];
  message[8] = '\0';

  // Send text typed in the console over the can network
  struct can_frame send_frame;
  int read = Serial.readBytesUntil('\n', send_frame.data, 8);
  if (read > 0) {
    send_frame.can_id = id;
    send_frame.can_dlc = read;
    mcp2515.sendMessage(&send_frame);
    Serial.println("sent");
  }

  // Read from the can bus
  struct can_frame receive_frame;
  if (mcp2515.readMessage(&receive_frame) == MCP2515::ERROR_OK) {
    Serial.print("[prio-bit: ");
    Serial.print(receive_frame.can_id >> 10 & 0b00000001);
    Serial.print("\tmodule-type: ");
    Serial.print(receive_frame.can_id >> 8 & 0b00000011);
    Serial.print("\tmodule-id: ");

    Serial.print(receive_frame.can_id & 0b11111111);
    Serial.print("]\tLength: ");
    
    Serial.print(receive_frame.can_dlc, HEX); // print DLC
    Serial.print("\t Body: ");

    // First byte has the type of the message

    // print per byte in hex
    for (int i = 0; i < receive_frame.can_dlc; i++)  { // print the data
      Serial.print(receive_frame.data[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");

    // print all
    //memcpy(message, receive_frame.data, 8);
    //Serial.println(message);
  }
}

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void play_melody() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(soundpin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(soundpin);
  }
}

void play_sound() {

}
