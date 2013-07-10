#pragma once
#include "stdafx.h"

// HD04408 LCD module with 4 bit data bus
class LCD {
public:
  enum Command //command bytes for LCD
  {
    CLEAR    = 0x01, // 0000.0001   Clear display
    HOME     = 0x02, // 0000.001-   Return home
    ENTRY    = 0x04, // 0000.01IS   Entry mode (I - Increment, S - Display shift)
    DISPLAY  = 0x08, // 0000.1DCB   Display control (D - Display on|off, C - Cursor on|off, B - Blinking on|off)
    SHIFT    = 0x10, // 0001.SR--   Cursor/Display shift (S - 0:Cursor/1:Screen, R - 0:Left/1:Right)
    FUNCTION = 0x20, // 001D.NF--   Function set (D - 4/8-bit, N - 1/2 lines, F - 5x8/5x10 dots)
    SETCGRAM = 0x40, // 01CC.CCCC   Set GCRAM (C - 3Fh address)
    SETDDRAM = 0x80, // 1DDD.DDDD   Set DDRAM (D - 7Fh address)
  };

  enum EntryOption // options for Command::ENTRY
  {
    INCREMENT     = 0x02,
    DECREMENT     = 0x00,
    DISPLAY_SHIFT = 0x01,
  };

  enum DisplayOption // options for Command::DISPLAY
  {
    DISPLAY_ON  = 0x04,
    DISPLAY_OFF = 0x00,
    CURSOR_ON   = 0x02,
    CURSOR_OFF  = 0x00,
    BLINK_ON    = 0x01,
    BLINK_OFF   = 0x00,
  };

  enum ShiftOption // options for Command::SHIFT
  {
    SCREEN  = 0x08,
    CURSOR  = 0x00,
    RIGHT   = 0x04,
    LEFT    = 0x00,
  };

  enum FunctionOption // options for Command::FUNCTION
  {
    DATA_8_BIT    = 0x10,
    DATA_4_BIT    = 0x00,
    TWO_LINES     = 0x08,
    ONE_LINE      = 0x00,
    CHAR_5x10_DOT = 0x04,
    CHAR_5x8_DOT  = 0x00,
  };

  enum DigitBase
  {
    BIN = 2,
    OCT = 8,
    DEC = 10,
    HEX = 16,
  };

public:
  static void commandWrite(uint8_t value);
  static void init();
  static void print(uint8_t value);
  static void printIn(const char* msg);
  static void printIn(const char* msg, uint8_t len);
  static void clear();
  static void home();
  //non-core---------------
  static void cursorTo(uint8_t line_num, uint8_t x);
  static void leftScroll(uint8_t chars, uint8_t delay_time);
  static void setDisplay(bool display, bool cursor, bool blink);
  static void setBacklight(uint8_t level);

  static void printDigit(uint8_t value);
  static void printDigit2(uint8_t value, DigitBase base = DEC);
  static void printDigit3(uint8_t value, DigitBase base = DEC);

  static void buildChar(uint8_t location, const uint8_t charmap[8]);
  static void buildChars(const uint8_t charmap[64]);
  //end of non-core--------

  //4bit only, therefore ideally private but may be needed by user
  static void commandWriteNibble(uint8_t nibble);
private:
  static void initBacklight();
  static void pulseEnablePin();
  static void pushNibble(uint8_t nibble);
  static void pushByte(uint8_t value);

private:
  static const uint8_t mLines = 2; // 1 or 2 lines
};
