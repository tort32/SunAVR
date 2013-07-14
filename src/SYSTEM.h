#pragma once
#include "stdafx.h"
#include "IR.h"
#include "RGBLED.h"
#include "LCD.h"
#include "RTC.h"
#include "colors.h"

class SYSTEM
{
public:
  enum
  {
    CMD_NONE = 0xFF,
    CMD_INIT = 0xFE,

    // LED IR Control Table
    ADR_LED_CONTROL = 0x00,
    CMD_UP = 0x09,
    CMD_DOWN = 0x1D,
    CMD_OFF = 0x1F,
    CMD_ON = 0x0D,
    CMD_R = 0x19,
    CMD_G = 0x1B,
    CMD_B = 0x11,
    CMD_W = 0x15,
    CMD_01 = 0x17,
    CMD_02 = 0x12,
    CMD_03 = 0x16,
    CMD_FLASH = 0x4D,
    CMD_04 = 0x40,
    CMD_05 = 0x4C,
    CMD_06 = 0x04,
    CMD_STROBE = 0x00,
    CMD_07 = 0x0A,
    CMD_08 = 0x1E,
    CMD_09 = 0x0E,
    CMD_FADE = 0x1A,
    CMD_10 = 0x1C,
    CMD_11 = 0x14,
    CMD_12 = 0x0F,
    CMD_SMOOTH = 0x0C,
  };
  enum
  {
    STATE_INPUT_CMD = 0x00,
    STATE_MASK = 0xF0, // HI bits for the state
    STATE_SUB_MASK = 0x0F, // LOW bits for the substate
    STATE_INPUT_TUNE = 0x10, // 2 low bits for component selection
    STATE_INPUT_TUNE_R = STATE_INPUT_TUNE | 0,
    STATE_INPUT_TUNE_G = STATE_INPUT_TUNE | 1,
    STATE_INPUT_TUNE_B = STATE_INPUT_TUNE | 2,
    STATE_INPUT_TUNE_MAX = STATE_INPUT_TUNE | 3,
    STATE_INPUT_HEX = 0x20, // 3 low bits for digit selection
    STATE_INPUT_HEX_1 = STATE_INPUT_HEX | 0,
    STATE_INPUT_HEX_2 = STATE_INPUT_HEX | 1,
    STATE_INPUT_HEX_3 = STATE_INPUT_HEX | 2,
    STATE_INPUT_HEX_4 = STATE_INPUT_HEX | 3,
    STATE_INPUT_HEX_5 = STATE_INPUT_HEX | 4,
    STATE_INPUT_HEX_6 = STATE_INPUT_HEX | 5,
    STATE_INPUT_HEX_MAX = STATE_INPUT_HEX | 6,
  };
public:
  static void init()
  {
     mRunCmd = CMD_ON;
     mState = STATE_INPUT_CMD;

     LCD::init();
     IR::init(&IRCallback);
     //TWI::Init();
     LED::init();
  }

  inline static void update()
  {
    uint8_t cnt = LED::update();
    if(cnt == 0)
    {
      checkCmd();
    }
  }
private:
  static void IRCallback(uint8_t adr, uint8_t cmd)
  {
    /*LCD::cursorTo(1,0);
    LCD::printIn("ADR=");
    LCD::printDigit2(adr, LCD::HEX);
    LCD::cursorTo(2,0);
    LCD::printIn("CMD=");
    LCD::printDigit2(cmd, LCD::HEX);*/
    if(adr == ADR_LED_CONTROL)
    {
      mRunCmd = cmd;
    }
  }

  static void checkCmd()
  {
    if(mRunCmd == CMD_NONE)
      return;

    LED::disable(); // Off leds

    if(mRunCmd == CMD_UP)
    {
      LED::incLevel();
    }
    else if(mRunCmd == CMD_DOWN)
    {
      LED::decLevel();
    }
    else
    {
      switch(mState & STATE_MASK)
      {
      case STATE_INPUT_CMD: CheckInputCmd(); break;
      case STATE_INPUT_HEX: CheckInputHex(); break;
      case STATE_INPUT_TUNE: CheckInputTune(); break;
      }

      LED::setColor(mColor);
    }

    CheckInputPost();

    mRunCmd = CMD_NONE;
  }

private:
  static void SetColorDigit(uint8_t digit, uint8_t val)
  {
    uint8_t comp = digit >> 1; // 0 - R, 1 - G, 2 - B
    uint8_t shift = (digit % 2) ? 0 : 4;
    uint8_t mask = 0x0F << shift; // 0xF0 - HI, 0x0F - LO
    wrtbits(*((uint8_t*)&mColor + comp), val<<shift, mask);
  }

  static void CheckInputCmd()
  {
    switch(mRunCmd)
    {
    case CMD_OFF: mColor = LED::Color(0,0,0); break;
    case CMD_ON:  mColor = LED::Color(0xFF,0xFF,0xFF); break;
    case CMD_R:   mColor = gColorTable[0]; break;
    case CMD_G:   mColor = gColorTable[1]; break;
    case CMD_B:   mColor = gColorTable[2]; break;
    case CMD_W:   mColor = gColorTable[3]; break;
    case CMD_01:  mColor = gColorTable[4]; break;
    case CMD_02:  mColor = gColorTable[5]; break;
    case CMD_03:  mColor = gColorTable[6]; break;
    case CMD_04:  mColor = gColorTable[7]; break;
    case CMD_05:  mColor = gColorTable[8]; break;
    case CMD_06:  mColor = gColorTable[9]; break;
    case CMD_07:  mColor = gColorTable[10]; break;
    case CMD_08:  mColor = gColorTable[11]; break;
    case CMD_09:  mColor = gColorTable[12]; break;
    case CMD_10:  mColor = gColorTable[13]; break;
    case CMD_11:  mColor = gColorTable[14]; break;
    case CMD_12:  mColor = gColorTable[15]; break;
    case CMD_FADE:    mState = STATE_INPUT_HEX; break;
    case CMD_SMOOTH:  mState = STATE_INPUT_TUNE; break;
    }
  }

  static void CheckInputHex()
  {
    uint8_t digit = mState & STATE_SUB_MASK; // 6 digits (0..5)
    switch(mRunCmd)
    {
    case CMD_W: /*0*/ SetColorDigit(digit, 0x00); break;
    case CMD_R: /*1*/ SetColorDigit(digit, 0x01); break;
    case CMD_G: /*2*/ SetColorDigit(digit, 0x02); break;
    case CMD_B: /*3*/ SetColorDigit(digit, 0x03); break;
    case CMD_01:/*4*/ SetColorDigit(digit, 0x04); break;
    case CMD_02:/*5*/ SetColorDigit(digit, 0x05); break;
    case CMD_03:/*6*/ SetColorDigit(digit, 0x06); break;
    case CMD_04:/*7*/ SetColorDigit(digit, 0x07); break;
    case CMD_05:/*8*/ SetColorDigit(digit, 0x08); break;
    case CMD_06:/*9*/ SetColorDigit(digit, 0x09); break;
    case CMD_07:/*A*/ SetColorDigit(digit, 0x0A); break;
    case CMD_08:/*B*/ SetColorDigit(digit, 0x0B); break;
    case CMD_09:/*C*/ SetColorDigit(digit, 0x0C); break;
    case CMD_10:/*D*/ SetColorDigit(digit, 0x0D); break;
    case CMD_11:/*E*/ SetColorDigit(digit, 0x0E); break;
    case CMD_12:/*F*/ SetColorDigit(digit, 0x0F); break;
    case CMD_FLASH:   mState = STATE_INPUT_CMD; break;
    case CMD_SMOOTH:  mState = STATE_INPUT_TUNE; break;
    case CMD_FADE:
      if(++mState == STATE_INPUT_HEX_MAX)
        mState = STATE_INPUT_HEX;
      break;
    }
  }

  static void CheckInputTune()
  {
    uint8_t comp = mState & STATE_SUB_MASK;
    switch(mRunCmd)
    {
    case CMD_R: incByte(*((uint8_t*)&mColor + comp)); break;
    case CMD_G: decByte(*((uint8_t*)&mColor + comp)); break;
    case CMD_FLASH:   mState = STATE_INPUT_CMD; break;
    case CMD_FADE:    mState = STATE_INPUT_HEX; break;
    case CMD_SMOOTH:
      if(++mState == STATE_INPUT_TUNE_MAX)
        mState = STATE_INPUT_TUNE;
      break;
    }
  }

  static void CheckInputPost()
  {
    LED::Color level = LED::getLevel();

    LCD::cursorTo(1,0);
    LCD::printIn("PWM=");
    LCD::printDigit2(level.R, LCD::HEX);
    LCD::print('.');
    LCD::printDigit2(level.G, LCD::HEX);
    LCD::print('.');
    LCD::printDigit2(level.B, LCD::HEX);

    LCD::cursorTo(2,0);
    if(mState == STATE_INPUT_CMD)
    {
      LCD::printIn("NORMAL ");
    }
    else if(mState & STATE_INPUT_TUNE)
    {
      LCD::printIn("TUNE  ");
      switch(mState & STATE_SUB_MASK)
      {
      case 0: LCD::print('R'); break;
      case 1: LCD::print('G'); break;
      case 2: LCD::print('B'); break;
      }
    }
    else if(mState & STATE_INPUT_HEX)
    {
      LCD::printIn("INPUT ");
      LCD::print('0' | (mState & STATE_SUB_MASK));
    }
  }

private:
  static volatile uint8_t mRunCmd;
  static LED::Color mColor;
  static uint8_t mState;
};