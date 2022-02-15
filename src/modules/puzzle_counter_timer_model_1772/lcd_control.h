//
// lcd_control.h
//

#include <LedControl.h>

#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H

// Main LCD controller
extern LedControl lcd;

// What will be printed on the next call of lcd_draw()
// The digits of each binary number represent the following segments (first digit unused)
//  B0 x x x x x x x
//     A B C D E F G
// With each segment being positioned as follows:
//   A
// F   B
//   G
// E   C
//   D
extern byte LCD_STATE[6];

// Maps segment indices (A=0, B=1, ...) to their 'correct' hardware indices
// (pin 2 got its solder pad torn off :<)
extern uint8_t segment_map[7];

// Which segments to light up for each number from 0 to 9
// Indices correspond to their numbers
extern byte character_map[10];

// Writes the current value of LCD_STATE to the lcd
void lcd_draw();

// Set a single digit on the LCD
void lcd_set_digit(uint8_t digit, uint8_t segments);

// Set every digit on the LCD
void lcd_set_display(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6);

// Print a number to the LCD
//
// num should be in [-99999; 999999] (the display only has 6 digits)
//
// Exit codes:
// - 0: success
// - 1: num was out of bounds
uint8_t lcd_print_number(int32_t num);

#endif
