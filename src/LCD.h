#pragma once
#include "stdafx.h"

// HD04408 LCD module with 4 bit data bus
namespace LCD
{
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

  void init();
  //void commandWrite(uint8_t value);
  void print(uint8_t value);
  void printIn(const char* msg);
  void printIn(const char* msg, uint8_t len);
  void clear();
  void home();

  void cursorTo(uint8_t line_num, uint8_t x);
  void leftScroll(uint8_t chars, uint8_t delay_time);
  void setDisplay(bool display, bool cursor, bool blink);
  void setBacklight(uint8_t level);

  void printDigit(uint8_t value);
  void printDigit2(uint8_t value, DigitBase base = DEC);
  void printDigit3(uint16_t value, DigitBase base = DEC);

  void buildChar(uint8_t location, const uint8_t charmap[8]);
  void buildChars(const uint8_t charmap[64]);
};
