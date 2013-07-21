#pragma once
#include "stdafx.h"

#define LED_R _BV(PD0)
#define LED_G _BV(PD1)
#define LED_B _BV(PD2)
#define LED_PINS (LED_R|LED_G|LED_B)

#define LED_DDR  DDRD
#define LED_PORT PORTD

#define LED_LEVEL_MAX 7

//#define LED_PRECISE 1

namespace LED
{
  struct Color
  {
    union
    {
      struct
      {
        uint8_t R;
        uint8_t G;
        uint8_t B;
      };
      uint8_t v[3];
    };
    Color() {};
    Color(uint8_t red, uint8_t green, uint8_t blue): R(red), G(green), B(blue) {};
    inline Color operator>>(uint8_t level) { return Color(R >> level, G >> level, B >> level); }
  };

  void init();
  void disable();
  void setColor(const Color& color);
  void setLevel(uint8_t level);
  void incLevel();
  void decLevel();
  Color getLevel();
  Color getPreScl();

  inline uint8_t update()
  {
    extern uint8_t mCounter;
    extern Color mValue;
#ifdef LED_PRECISE
    extern Color mPreScl;
    extern Color mPreSclCnt;
    if(mCounter == mValue.R && mPreSclCnt.R == 0) clrbits(LED_PORT, LED_R); // OFF
    if(mCounter == mValue.G && mPreSclCnt.G == 0) clrbits(LED_PORT, LED_G); // OFF
    if(mCounter == mValue.B && mPreSclCnt.B == 0) clrbits(LED_PORT, LED_B); // OFF
    if(mCounter == 0 && mValue.R != 0 && mPreSclCnt.R == 0) setbits(LED_PORT, LED_R); // ON
    if(mCounter == 0 && mValue.G != 0 && mPreSclCnt.G == 0) setbits(LED_PORT, LED_G); // ON
    if(mCounter == 0 && mValue.B != 0 && mPreSclCnt.B == 0) setbits(LED_PORT, LED_B); // ON
#else
    if(mCounter == mValue.R) clrbits(LED_PORT, LED_R); // OFF
    if(mCounter == mValue.G) clrbits(LED_PORT, LED_G); // OFF
    if(mCounter == mValue.B) clrbits(LED_PORT, LED_B); // OFF
    if(mCounter == 0 && mValue.R != 0) setbits(LED_PORT, LED_R); // ON
    if(mCounter == 0 && mValue.G != 0) setbits(LED_PORT, LED_G); // ON
    if(mCounter == 0 && mValue.B != 0) setbits(LED_PORT, LED_B); // ON
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
}
