//
// lcd_control.c
//

#include "lcd_control.h"

#include <LedControl.h>

#define LED_PIN_DIN 2 // Data In (MOSI)
#define LED_PIN_CS  3 // Chip Select
#define LED_PIN_CLK 5 // Clock

LedControl lcd = LedControl(LED_PIN_DIN, LED_PIN_CLK, LED_PIN_CS, 1);

byte LCD_STATE[6] = {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000};
uint8_t segment_map[7] = {0, 1, 3, 4, 5, 6, 7};
byte character_map[10] = {B01111110,B00110000,B01101101,B01111001,B00110011,B01011011,B01011111,B01110000,B01111111,B01111011};

void lcd_draw() {
	uint8_t digit_state;
	for (int i=0; i<6; i++) {
		digit_state = LCD_STATE[i];

		for (int j=6; j>=0; j--) {
			// Map j to missing-solder-pad adjusted value
			// i + 1 because only digits 1-6 of possible 0-7 are used
			// continually read the last digit of digit_state and shift it over by one
			lcd.setLed(0, segment_map[j], i + 1, digit_state % 2);
			digit_state = digit_state >> 1;
		}
	}
}

void lcd_set_digit(uint8_t digit, uint8_t segments) {
	LCD_STATE[digit] = segments;

	lcd_draw();
}

void lcd_set_display(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6) {
	LCD_STATE[0] = d1;
	LCD_STATE[1] = d2;
	LCD_STATE[2] = d3;
	LCD_STATE[3] = d4;
	LCD_STATE[4] = d5;
	LCD_STATE[5] = d6;

	lcd_draw();
}

uint8_t lcd_print_number(int32_t num) {
	if (num < -99999 || num > 999999) {
		return 1;
	}

	// Normally leading zeroes will be ignored,
	// but if the number is exactly zero it should be printed
	if (num == 0) {
		lcd_set_digit(5, B01111110);
		return 0;
	}

	bool negative = false;
	if (num < 0) {
		negative = true;

		// Change from negative to positive in two's complement
		// Faster than doing num * -1
		num = num ^ 0xFFFFFFFF;
		num = num + 1;
	}

	uint8_t index;
	for (int8_t i=5; i>=0; i--) {
		// Continually read the last decimal and then shift
		// num over by one decimal
		LCD_STATE[i] = character_map[num % 10];
		num /= 10;

		index = i;
		if (num == 0) break;
	}

	// Draw a minus symbol if the number was negative
	if (negative) LCD_STATE[index - 1] = B00000001;

	lcd_draw();

	return 0;
}
