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
public:
  static void init()
  {
     mRunCmd = CMD_NONE;

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
    if(adr == 0x00)
    {
      mRunCmd = cmd;
    }
  }

  inline static void checkCmd()
  {
    if(mRunCmd != CMD_NONE)
    {
      LED::disable(); // Off leds

      switch(mRunCmd)
      {
      case CMD_OFF:   LED::setColor(LED::Color(0,0,0)); break;
      case CMD_ON:    LED::setColor(LED::Color(0xFF,0xFF,0xFF)); break;
      case CMD_UP:    LED::incLevel(); break;
      case CMD_DOWN:  LED::decLevel(); break;
      case CMD_R:     LED::setColor(gColorTable[0]); break;
      case CMD_G:     LED::setColor(gColorTable[1]); break;
      case CMD_B:     LED::setColor(gColorTable[2]); break;
      case CMD_W:     LED::setColor(gColorTable[3]); break;
      case CMD_01:    LED::setColor(gColorTable[4]); break;
      case CMD_02:    LED::setColor(gColorTable[5]); break;
      case CMD_03:    LED::setColor(gColorTable[6]); break;
      case CMD_04:    LED::setColor(gColorTable[7]); break;
      case CMD_05:    LED::setColor(gColorTable[8]); break;
      case CMD_06:    LED::setColor(gColorTable[9]); break;
      case CMD_07:    LED::setColor(gColorTable[10]); break;
      case CMD_08:    LED::setColor(gColorTable[11]); break;
      case CMD_09:    LED::setColor(gColorTable[12]); break;
      case CMD_10:    LED::setColor(gColorTable[13]); break;
      case CMD_11:    LED::setColor(gColorTable[14]); break;
      case CMD_12:    LED::setColor(gColorTable[15]); break;
      }

      LED::Color level = LED::getLevel();
      LCD::cursorTo(1,0);
      LCD::printIn("PWM=");
      LCD::printDigit2(level.R, LCD::HEX);
      LCD::print('.');
      LCD::printDigit2(level.G, LCD::HEX);
      LCD::print('.');
      LCD::printDigit2(level.B, LCD::HEX);

      mRunCmd = CMD_NONE;
    }
  }
private:
  static volatile uint8_t mRunCmd;
  static LED::Color mColor;
  static uint8_t mState;
};