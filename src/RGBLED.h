#pragma once
#include "stdafx.h"

#define LED_R _BV(PD0)
#define LED_G _BV(PD1)
#define LED_B _BV(PD2)
#define LED_PINS (LED_R|LED_G|LED_B)

#define LED_PORT PORTD

#define LED_LEVEL_MAX 7

//#define LED_PRECISE 1

class LED
{
public:
  struct Color
  {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    Color() {};
    Color(uint8_t red, uint8_t green, uint8_t blue): R(red), G(green), B(blue) {};
    inline Color operator>>(uint8_t level) { return Color(R >> level, G >> level, B >> level); }
    inline uint8_t& getComponent(uint8_t comp) { return *((uint8_t*)this + comp); };
  };
public:
  static void init()
  {
    mColor = Color(0xFF,0xFF,0xFF);
    mLevel = 4;
    mCounter = 0;
    updatePrescalers();

    setbits(DDRD, LED_PINS); // output
  }

  static void disable()
  {
    // SHUT ALL OFF
    clrbits(PORTD, LED_PINS);
  }

  static inline uint8_t update()
  {
#ifdef LED_PRECISE
    if(mCounter == mValue.R && mPreSclCnt.R == 0) clrbits(PORTD, LED_R); // OFF
    if(mCounter == mValue.G && mPreSclCnt.G == 0) clrbits(PORTD, LED_G); // OFF
    if(mCounter == mValue.B && mPreSclCnt.B == 0) clrbits(PORTD, LED_B); // OFF
    if(mCounter == 0 && mValue.R != 0 && mPreSclCnt.R == 0) setbits(PORTD, LED_R); // ON
    if(mCounter == 0 && mValue.G != 0 && mPreSclCnt.G == 0) setbits(PORTD, LED_G); // ON
    if(mCounter == 0 && mValue.B != 0 && mPreSclCnt.B == 0) setbits(PORTD, LED_B); // ON
#else
    if(mCounter == mValue.R) clrbits(PORTD, LED_R); // OFF
    if(mCounter == mValue.G) clrbits(PORTD, LED_G); // OFF
    if(mCounter == mValue.B) clrbits(PORTD, LED_B); // OFF
    if(mCounter == 0 && mValue.R != 0) setbits(PORTD, LED_R); // ON
    if(mCounter == 0 && mValue.G != 0) setbits(PORTD, LED_G); // ON
    if(mCounter == 0 && mValue.B != 0) setbits(PORTD, LED_B); // ON
#endif

    ++mCounter;
#ifdef LED_PRECISE
    if(mCounter == 0)
    {
      if(++mPreSclCnt.R >= mPreScl.R) mPreSclCnt.R = 0;
      if(++mPreSclCnt.G >= mPreScl.G) mPreSclCnt.G = 0;
      if(++mPreSclCnt.B >= mPreScl.B) mPreSclCnt.B = 0;
    }
#endif
    return mCounter;
  }

  static void setColor(const Color& color)
  {
    mColor = color;
    updatePrescalers();
  }

  static void setLevel(uint8_t level)
  {
    mLevel = level;
    updatePrescalers();
  }

  static void incLevel()
  {
    if(mLevel !=0 )
    {
      --mLevel;
      updatePrescalers();
    }
  }

  static void decLevel()
  {
    if(mLevel != LED_LEVEL_MAX)
    {
      ++mLevel;
      updatePrescalers();
    }
  }

  static inline Color getLevel()
  {
    return mValue;
  }

  static inline Color getPreScl()
  {
#ifdef LED_PRECISE
    return mPreScl;
#else
    return Color(0,0,0);
#endif
  }

  static void updatePrescalers()
  {
#ifdef LED_PRECISE
    for(uint8_t i = 0; i < 3; ++i)
    {
      uint8_t val = mColor.getComponent(i) >> mLevel;
      if(val >= 0x10)
      {
        mValue.getComponent(i) = val;
        mPreScl.getComponent(i) = 1;
      }
      else if(val >= 0x08)
      {
        mValue.getComponent(i) = mColor.getComponent(i) >> (mLevel - 1);
        mPreScl.getComponent(i) = 2;
      }
      else if(val >= 0x04)
      {
        mValue.getComponent(i) = mColor.getComponent(i) >> (mLevel - 2);
        mPreScl.getComponent(i) = 4;
      }
      else
      {
        mValue.getComponent(i) = mColor.getComponent(i) >> (mLevel - 3);
        mPreScl.getComponent(i) = 8;
      }
    }
    mPreSclCnt = Color(0,0,0);
#else
    mValue = mColor >> mLevel;
#endif
  }

private:
  static uint8_t mCounter;
  static Color mColor;
  static Color mValue;
#ifdef LED_PRECISE
  static Color mPreScl;
  static Color mPreSclCnt;
#endif
  static uint8_t mLevel; // brightness factor
};