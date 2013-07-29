#include "stdafx.h"
#include "LCD.h"
#include "RGBLED.h"
#include "IR.h"
#include "RTC.h"
#include "colors.h"

namespace SYSTEM
{
  volatile uint8_t mRunCmd;
  LED::Color mColor;
  uint8_t mInputState;
  uint8_t mAnimState;
  uint8_t mAnimSpeed;
  uint16_t mAnimCnt;

  // Animation counter updates in 300Hz
  //const uint32_t ANIM_COUNTER_MAX = 256 * 256; // Should be the power of 2 to generate effective code
#define ANIM_COUNTER_MAX 0x4000
  const uint8_t ANIM_SPEED_LEVEL_MAX = 8;
  const uint8_t mAnimSpeedTable[ANIM_SPEED_LEVEL_MAX] = {1, 2, 3, 5, 7, 10, 14, 20};

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
  };

  enum
  {
    STATE_ANIM_NONE = 0x00,
    STATE_ANIM_FLASH,
    STATE_ANIM_STROBE,
    STATE_ANIM_FADE,
    STATE_ANIM_SMOOTH,
  };

  void CheckInputCmd();
  void CheckInputHex();
  void CheckInputTune();
  void CheckInputPost();
  void IRCallback(uint8_t adr, uint8_t cmd);

  void init()
  {
     mRunCmd = STATE_ANIM_FADE; //CMD_OFF;
     mInputState = STATE_INPUT_CMD;
     mAnimState = STATE_ANIM_NONE;
     mAnimSpeed = 0;

     LCD::init();
     IR::init(&IRCallback);
     //TWI::Init();
     LED::init();

     mColor = LED::Color(0xFF,0xFF,0xFF);
     LED::setColor(mColor);
     LED::setLevel(0);

     setbits(DDRD, _BV(PD6) | _BV(PD7));
  }

  void updateAnimFlash()
  {
    // Switch colors in a sequence
    const uint8_t colorSeq[8] = {3, 14, 15, 2, 3, 13, 1, 0};
    const uint16_t seqLength = ANIM_COUNTER_MAX / 8;
    uint16_t seqPart = mAnimCnt / seqLength; // 0..7
    LED::setColor(gColorTable[colorSeq[seqPart]]);
  }

  void updateAnimStrobe()
  {
    // Fast burst the light
    uint16_t cnt = mAnimCnt % 512;
    if(cnt<20)
      LED::setColor(mColor);
    else
      LED::setColor(LED::Color(0,0,0));
  }

  void updateAnimFade()
  {
    // Fade light up and down
    uint8_t semilevel; // 0..15
    uint16_t cnt = mAnimCnt % (ANIM_COUNTER_MAX / 8); // make faster in 8 times
    const uint16_t halfAnim = ANIM_COUNTER_MAX / (2 * 8);
    if(cnt < halfAnim)
    {
      semilevel = (cnt / (halfAnim / 16)); // 0..15
    }
    else
    {
      semilevel = 16 - (cnt - halfAnim) / (halfAnim / 16); // 15..0
    }
    LED::setColor(mColor.attenuate(semilevel));
  }

  void updateAnimSmooth()
  {
    // Whole animation slitted on 8 segments
    // Each segment is a color interpolation
    const uint8_t colorSeq[9] = {0, 13, 1, 14, 2, 15, 1, 2, 0};
    const uint16_t seqLength = ANIM_COUNTER_MAX / 8;
    uint16_t seqPart = mAnimCnt / seqLength; // 0..7
    uint16_t seqFract = mAnimCnt % seqLength; // 0..(seqLength-1)
    uint8_t semilevel = seqFract / (seqLength / 16); // 0..15
    LED::setColor(gColorTable[colorSeq[seqPart]].interpolateLinear(gColorTable[colorSeq[seqPart+1]], semilevel));
  }

  uint16_t UpdateAnim()
  {
    mAnimCnt += mAnimSpeedTable[mAnimSpeed];

#if ANIM_COUNTER_MAX != 0x10000
    if(mAnimCnt > ANIM_COUNTER_MAX)
      mAnimCnt -= ANIM_COUNTER_MAX; // Reset counter
#endif

    switch(mAnimState)
    {
    case STATE_ANIM_NONE:   break;
    case STATE_ANIM_FLASH:  updateAnimFlash(); break;
    case STATE_ANIM_STROBE: updateAnimStrobe(); break;
    case STATE_ANIM_FADE:   updateAnimFade(); break;
    case STATE_ANIM_SMOOTH: updateAnimSmooth(); break;
    }

    return mAnimCnt;
  }

  void ReadClock()
  {
    setbits(PORTD, _BV(PD6));
    RTC::read(); // 220us at 8MHz
    clrbits(PORTD, _BV(PD6));
  }

  void UpdateClock()
  {
    setbits(PORTD, _BV(PD7));
    LCD::cursorTo(2,8);
    LCD::printDigit(RTC::get(RTC::HOUR_HI));
    LCD::printDigit(RTC::get(RTC::HOUR_LO));
    LCD::print(':');
    LCD::printDigit(RTC::get(RTC::MIN_HI));
    LCD::printDigit(RTC::get(RTC::MIN_LO));
    LCD::print(':');
    LCD::printDigit(RTC::get(RTC::SEC_HI));
    LCD::printDigit(RTC::get(RTC::SEC_LO));
    clrbits(PORTD, _BV(PD7));
  }

  void IRCallback(uint8_t adr, uint8_t cmd)
  {
    if(adr == ADR_LED_CONTROL)
    {
      mRunCmd = cmd;
    }
  }

  void checkCmd()
  {
    if(mRunCmd == CMD_NONE)
      return;

    LED::disable(); // Off leds

    if(mRunCmd == CMD_UP)
    {
      if(mAnimState == STATE_ANIM_NONE)
        LED::incLevel();
      else
        incByte(mAnimSpeed, ANIM_SPEED_LEVEL_MAX - 1);
    }
    else if(mRunCmd == CMD_DOWN)
    {
      if(mAnimState == STATE_ANIM_NONE)
        LED::decLevel();
      else
        decByte(mAnimSpeed, 0);
    }
    else
    {
      switch(mInputState & STATE_MASK)
      {
      case STATE_INPUT_CMD: CheckInputCmd(); break;
      }

      LED::setColor(mColor);
    }

    CheckInputPost();

    mRunCmd = CMD_NONE;
  }

  void CheckInputCmd()
  {
    switch(mRunCmd)
    {
    case CMD_OFF: mColor = LED::Color(0,0,0); mAnimState = STATE_ANIM_NONE; break;
    case CMD_ON:  mColor = LED::Color(0xFF,0xFF,0xFF); mAnimState = STATE_ANIM_NONE; break;
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
    case CMD_FLASH:   mAnimState = (mAnimState != STATE_ANIM_FLASH) ? STATE_ANIM_FLASH : STATE_ANIM_NONE; break;
    case CMD_STROBE:  mAnimState = (mAnimState != STATE_ANIM_STROBE) ? STATE_ANIM_STROBE : STATE_ANIM_NONE; break;
    case CMD_FADE:    mAnimState = (mAnimState != STATE_ANIM_FADE) ? STATE_ANIM_FADE : STATE_ANIM_NONE; break;
    case CMD_SMOOTH:  mAnimState = (mAnimState != STATE_ANIM_SMOOTH) ? STATE_ANIM_SMOOTH : STATE_ANIM_NONE; break;
    }
  }

  void CheckInputPost()
  {
    const LED::Color& level = LED::getLevel();
    LCD::cursorTo(1,0);
    LCD::printIn("PWM=");
    LCD::printDigit3(level.R, LCD::HEX);
    LCD::print('.');
    LCD::printDigit3(level.G, LCD::HEX);
    LCD::print('.');
    LCD::printDigit3(level.B, LCD::HEX);

    LCD::cursorTo(2,0);
    switch(mAnimState)
    {
    case STATE_ANIM_NONE:   LCD::printIn("NONE  "); break;
    case STATE_ANIM_FLASH:  LCD::printIn("FLASH "); break;
    case STATE_ANIM_STROBE: LCD::printIn("STROBE"); break;
    case STATE_ANIM_FADE:   LCD::printIn("FADE  "); break;
    case STATE_ANIM_SMOOTH: LCD::printIn("SMOOTH"); break;
    }
  }
}
