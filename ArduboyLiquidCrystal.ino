/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include "LiquidCrystal.h"
#include <Arduboy2.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to:
const uint8_t rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Arduboy2 arduboy;

int thisChar = 'A';

void setup() {
  arduboy.boot(); // raw hardware
  arduboy.display(); // blank the display

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  delay(1000);
}

void loop() {
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.print(thisChar);
    lcd.draw();
    delay(500);
  }
  // set the cursor to (16,1):
  lcd.setCursor(16, 1);
  // set the display to automatically scroll:
  lcd.autoscroll();
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.print(thisChar);
    lcd.draw();
    delay(500);
  }
  // turn off automatic scrolling:
  lcd.noAutoscroll();
  // clear screen:
  lcd.clear();

  // turn on the cursor:
  lcd.cursor();
  while (thisChar <= 'Z') {
    // reverse directions at 'm':
    if (thisChar == 'M') {
      // go right for the next letter:
      lcd.rightToLeft();
    }
    // reverse again at 's':
    if (thisChar == 'S') {
      // go left for the next letter:
      lcd.leftToRight();
    }
    // print the character:
    lcd.write(thisChar);
    lcd.draw();
    // wait half a second:
    delay(500);
    // increment the letter:
    thisChar++;
  }
  // turn off the cursor:
  lcd.noCursor();

  // clear screen:
  lcd.clear();
  // Print a message to the LCD:
  lcd.print("HELLO, WORLD!");
  lcd.draw();
  delay(500);
  // scroll 13 positions (string length) to the left
  // to move it offscreen left:
  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    lcd.draw();
    // wait a bit:
    delay(150);
  }
  delay(500);
  // scroll 29 positions (string length + display length) to the right
  // to move it offscreen right:
  for (int positionCounter = 0; positionCounter < 29; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    lcd.draw();
    // wait a bit:
    delay(150);
  }
  delay(500);
  // scroll 16 positions (display length + string length) to the left
  // to move it back to center:
  for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    lcd.draw();
    // wait a bit:
    delay(150);
  }

  // turn on blinking cursor:
  lcd.cursor();
  lcd.blink();
  while (1) {
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis() / 1000);
    lcd.draw();
  }
}

