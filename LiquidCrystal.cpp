#include "LiquidCrystal.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
#include "Retro8x16.h"

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
           uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
           uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  begin(16, 2);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable,
           uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
           uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  begin(16, 2);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
           uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  begin(16, 2);
}

LiquidCrystal::LiquidCrystal(uint8_t rs,  uint8_t enable,
           uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  begin(16, 2);
}

void LiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
       uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
       uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  begin(16, 2);
}

volatile uint8_t cursorBlink = 0;

ISR(TIMER3_COMPA_vect)
{
  cursorBlink ^= B00000001;
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  // Set up Timer 3 for cursorBlink interrupt
  TCCR3A = 0; // Normal operation
  TCCR3B = _BV(WGM32) | _BV(CS30) | _BV(CS32); // CTC, scale to clock ÷ 1024
  OCR3A = 8192; // Compare A register value
  TIMSK3 = _BV(OCIE3A); // Interrupt on compare A match

  RAM_current = DDR;
  DDRAM_counter = 0;
  DDRAM_display = 0;
  // CGRAM_counter = 0;
  LCD_CursorPos = 255;

  // Init DDRAM
  for (uint8_t i = 0; i < 104; i++)
    DDRAM[i] = 0x00;

  DDRAM[40] = 95; // 127 (Cursor) - 32

  _numlines = lines;
  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  command(LCD_ENTRYMODESET | _displaymode); // set the entry mode
}

void LiquidCrystal::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void LiquidCrystal::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
}

void LiquidCrystal::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/0
  }

  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) {
  /*
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
  */
}

/*********** mid level commands, for sending data/cmds */

// adapted from https://github.com/dylangageot/LCDSim
inline void LiquidCrystal::command(uint8_t instruction) {
  uint8_t i;
  for (i = 0; i < 8; i++)
      if (instruction & (0x80 >> i))
          break;
  switch (i) {
      // SET DDRAM ADDRESS
      case 0:
          DDRAM_counter = instruction & 0x7F;
          RAM_current = DDR;
          break;
      // SET CGRAM ADDRESS
      case 1:
          // CGRAM_counter = instruction & 0x3F;
          RAM_current = CGR;
          break;
      // CURSOR/DISPLAY SHIFT
      case 3:
          if (instruction & 0x08) {
              if (instruction & 0x04) {
                  if (DDRAM_display == 0)
                      DDRAM_display = 39;
                  else
                      DDRAM_display--;
              } else {
                  if (DDRAM_display == 39)
                      DDRAM_display = 0;
                  else
                      DDRAM_display++;
              }
          } else {
              if (instruction & 0x04) {
                  if (DDRAM_counter < 103) {
                      if (DDRAM_counter == 39)
                          DDRAM_counter = 64;
                      else
                          DDRAM_counter++;
                  } else {
                      DDRAM_counter = 0;
                  }
              } else {
                  if (DDRAM_counter > 0) {
                      if (DDRAM_counter == 64)
                          DDRAM_counter = 39;
                      else
                          DDRAM_counter--;
                  } else {
                      DDRAM_counter = 103;
                  }
              }
          }
          break;
      // DISPLAY ON/OFF CONTROL
      case 4:
          LCD_CursorBlink = instruction & 0x01;
          LCD_CursorEnable = (instruction & 0x02) >> 1;
          LCD_DisplayEnable = (instruction & 0x04) >> 2;
          break;
      // ENTRY MODE SET
      case 5:
          LCD_EntryMode = instruction & 0x03;
          break;
      // HOME
      case 6:
          DDRAM_counter = 0;
          DDRAM_display = 0;
          break;
      // CLEAR
      case 7:
          for (i = 0; i < 40; i++)
              DDRAM[i] = 0x00;
          for (i = 64; i < 104; i++)
              DDRAM[i] = 0x00;
          DDRAM_counter = 0;
          DDRAM_display = 0;
          break;
  }
}

// adapted from https://github.com/dylangageot/LCDSim
inline size_t LiquidCrystal::write(uint8_t value) {
  if (RAM_current == DDR) { /* == CGR) {
      n = CGRAM_counter / 8;
      m = CGRAM_counter % 8;
      CGROM[n][m] = instruction & 0xFF;
      if (CGRAM_counter < 64)
          CGRAM_counter++;
  } else { */
      DDRAM[DDRAM_counter] = value - 32;
      if (LCD_EntryMode & 0x02) {
          if (DDRAM_counter < 103) {
              if (DDRAM_counter == 39)
                  DDRAM_counter = 64;
              else
                  DDRAM_counter++;
          } else {
              DDRAM_counter = 0;
          }
          if (LCD_EntryMode & 0x01) {
              if (DDRAM_display == 39)
                  DDRAM_display = 0;
              else
                  DDRAM_display++;
          }
      } else {
          if (DDRAM_counter > 0) {
              if (DDRAM_counter == 64)
                  DDRAM_counter = 39;
              else
                  DDRAM_counter--;
          } else {
              DDRAM_counter = 103;
          }
          if (LCD_EntryMode & 0x01) {
              if (DDRAM_display == 0)
                  DDRAM_display = 39;
              else
                  DDRAM_display--;
          }
      }
  }
  return 1; // assume success
}

void LiquidCrystal::draw() {
  // only writing to the first 4 'pages' of the Arduboy screen
  PORTD &= ~B00010000;            // LCDCommandMode
  SPDR = 0xB0;                    // set page address
  while (!(SPSR & _BV(SPIF))) { } // wait for byte to be sent
  SPDR = 0x10;                    // set higher column address
  while (!(SPSR & _BV(SPIF))) { } // wait for byte to be sent
  PORTD |= B00010000;             // LCDDataMode

  if (!LCD_DisplayEnable)
  {
    for (uint16_t n = 0; n < 512; n++)
    {
      SPDR = 0x00;
      while (!(SPSR & _BV(SPIF))) { } // wait for byte to be sent
    }
    return;
  }

  if (LCD_DisplayEnable && LCD_CursorEnable && (cursorBlink || LCD_CursorBlink == LCD_BLINKOFF))
  {
    if ((DDRAM_display <= DDRAM_counter) && ((DDRAM_display + 0x0F) >= DDRAM_counter))
      LCD_CursorPos = DDRAM_counter - DDRAM_display;
    if ((0x40 + DDRAM_display <= DDRAM_counter) && ((DDRAM_display + 0x4F) >= DDRAM_counter))
      LCD_CursorPos = DDRAM_counter - DDRAM_display;
  }

  // adapted from C++ version of Arduboy2Core::paintScreen()
  uint8_t d, j;
  uint8_t q = 0;

  for (uint8_t n = DDRAM_display; n < DDRAM_display + 16; n++)
  {
    if (n > 39) j = n - 40;
    else j = n;
    if (j == LCD_CursorPos) j = 40;

    // set the first SPI data byte to get things started
    SPDR = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

    // the code to iterate the loop and get the next byte from the buffer is
    // executed while the previous byte is being sent out by the SPI controller
    while (q < 8)
    {
      // get the next byte. It's put in a local variable so it can be sent as
      // as soon as possible after the sending of the previous byte has completed
      d = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

      while (!(SPSR & _BV(SPIF))) { } // wait for the previous byte to be sent

      // put the next byte in the SPI data register. The SPI controller will
      // clock it out while the loop continues and gets the next byte ready
      SPDR = d;
    }

    q = 0;

    while (!(SPSR & _BV(SPIF))) { } // wait for the last byte to be sent
  }

  q = 8;

  for (uint8_t n = DDRAM_display; n < DDRAM_display + 16; n++)
  {
    if (n > 39) j = n - 40;
    else j = n;
    if (j == LCD_CursorPos) j = 40;

    // set the first SPI data byte to get things started
    SPDR = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

    // the code to iterate the loop and get the next byte from the buffer is
    // executed while the previous byte is being sent out by the SPI controller
    while (q < 16)
    {
      // get the next byte. It's put in a local variable so it can be sent as
      // as soon as possible after the sending of the previous byte has completed
      d = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

      while (!(SPSR & _BV(SPIF))) { } // wait for the previous byte to be sent

      // put the next byte in the SPI data register. The SPI controller will
      // clock it out while the loop continues and gets the next byte ready
      SPDR = d;
    }

    q = 8;

    while (!(SPSR & _BV(SPIF))) { } // wait for the last byte to be sent
  }

  q = 0;

  for (uint8_t n = DDRAM_display + 64; n < DDRAM_display + 80; n++)
  {
    if (n > 103) j = n - 40;
    else j = n;
    if (j == LCD_CursorPos) j = 40;

    // set the first SPI data byte to get things started
    SPDR = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

    // the code to iterate the loop and get the next byte from the buffer is
    // executed while the previous byte is being sent out by the SPI controller
    while (q < 8)
    {
      // get the next byte. It's put in a local variable so it can be sent as
      // as soon as possible after the sending of the previous byte has completed
      d = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

      while (!(SPSR & _BV(SPIF))) { } // wait for the previous byte to be sent

      // put the next byte in the SPI data register. The SPI controller will
      // clock it out while the loop continues and gets the next byte ready
      SPDR = d;
    }

    q = 0;

    while (!(SPSR & _BV(SPIF))) { } // wait for the last byte to be sent
  }

  q = 8;

  for (uint8_t n = DDRAM_display + 64; n < DDRAM_display + 80; n++)
  {
    if (n > 103) j = n - 40;
    else j = n;
    if (j == LCD_CursorPos) j = 40;

    // set the first SPI data byte to get things started
    SPDR = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

    // the code to iterate the loop and get the next byte from the buffer is
    // executed while the previous byte is being sent out by the SPI controller
    while (q < 16)
    {
      // get the next byte. It's put in a local variable so it can be sent as
      // as soon as possible after the sending of the previous byte has completed
      d = pgm_read_byte(&Retro8x16[DDRAM[j]][q++]);

      while (!(SPSR & _BV(SPIF))) { } // wait for the previous byte to be sent

      // put the next byte in the SPI data register. The SPI controller will
      // clock it out while the loop continues and gets the next byte ready
      SPDR = d;
    }

    q = 8;

    while (!(SPSR & _BV(SPIF))) { } // wait for the last byte to be sent
  }

  LCD_CursorPos = 255;
}
