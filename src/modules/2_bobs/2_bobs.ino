/*
  Project Name: TM1638
  File: TM1638plus_HELLOWORLD_TEST_Model1.ino
  Description: A demo file library for TM1638 module(LED & KEY). Works on Model 1 and 3
  Carries out series most basic test , "hello world" helo wrld
  Author: Gavin Lyons.
  Created: feb 2020
  URL: https://github.com/gavinlyonsrepo/TM1638plus
*/
#include "./words.h"

#include <TM1638plus.h> //include the library

#define  STROBE_TM 2  // strobe = GPIO connected to strobe line of module
#define  CLOCK_TM 3  // clock = GPIO connected to clock line of module
#define  DIO_TM 5 // data = GPIO connected to data line of module
 
// Constructor object (GPIO STB , GPIO CLOCK , GPIO DIO, use high freq MCU)
TM1638plus tm(STROBE_TM, CLOCK_TM , DIO_TM, false);
int WORD_COUNT = 141;

static const unsigned char BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};


void setup()
{
      Serial.begin(9600);

    randomSeed(analogRead(0));
  tm.displayBegin();
  tm.brightness(0x50);
}

char count_bits(uint8_t v) {
  return BitsSetTable256[v & 0xff] + 
    BitsSetTable256[(v >> 8) & 0xff];
}

char to_upper(char c) {
  if(c < 'a') return c;
  return c - 32;
}
int expected_score = 0;
int guess = 0;

void set_score(const char* word) {
  expected_score = 0;
  for(int i =0; i < strlen(word); i++) {
     int c = (int) to_upper(word[i]) - 'A';
     expected_score += c;
  }
}

char get_input = 0;
char new_word = 1;
long index = 0;
uint8_t input = 0;

int at = WORD_COUNT;
char mword[10] = {};

uint8_t first_bit(uint8_t bits) {
  uint8_t out = 0;
  while(!(bits & 1)) {
    bits >>= 1;
    out += 1;
  }
  return out;
}

void handle_input() {
  uint8_t bits = count_bits(input);
  if(bits > 1) {
    Serial.println("good");
    Serial.println(guess == expected_score);
    if(guess == expected_score) {
      tm.setLED(0, 0xff);
      tm.setLED(1, 0xff);
      tm.setLED(2, 0xff);
      tm.setLED(3, 0xff);
      delay(5000);
    }
    new_word = 1;
  } else {
    uint8_t fb = first_bit(input);
    guess *= 8;
    guess += fb;
  }
  input = 0;
}

void loop()
{
  if(new_word) {
    new_word = 0;
    expected_score = 0;
    guess = 0;
    index = random(WORD_COUNT);
    set_score(words[index]);
    tm.reset();
    tm.displayText(words[index]);

    Serial.println(words[index]);
    Serial.println(expected_score);
    get_input = 1;
  }

  if (get_input) {
    uint8_t ni = tm.readButtons();
    input |= ni;
    if(input && ni == 0) handle_input();
  }
}
