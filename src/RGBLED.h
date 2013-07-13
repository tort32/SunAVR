#pragma once
#include "stdafx.h"

#define LED_R _BV(PD0)
#define LED_G _BV(PD1)
#define LED_B _BV(PD2)
#define LED_PINS (LED_R|LED_G|LED_B)

#define LED_PORT PORTD

#define LED_LEVEL_MAX 7

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
  };
public:
  static void init()
  {
    mLevel = 4;
    mCounter = 0;
    setbits(DDRD, LED_PINS); // output
  }

  static void disable()
  {
    // SHUT ALL OFF
    clrbits(PORTD, LED_PINS);
  }

  static inline uint8_t update()
  {
    Color pwm = mColor >> mLevel;
    if(mCounter == pwm.R) clrbits(PORTD, LED_R); // OFF
    if(mCounter == pwm.G) clrbits(PORTD, LED_G); // OFF
    if(mCounter == pwm.B) clrbits(PORTD, LED_B); // OFF
    if(mCounter == 0 && pwm.R != 0) setbits(PORTD, LED_R); // ON
    if(mCounter == 0 && pwm.G != 0) setbits(PORTD, LED_G); // ON
    if(mCounter == 0 && pwm.B != 0) setbits(PORTD, LED_B); // ON
    mCounter++;
    return mCounter;
  }

  static void setColor(const Color& color)
  {
    mColor = color;
  }

  static void setLevel(uint8_t level)
  {
    mLevel = level;
  }

  static void incLevel()
  {
    if(mLevel !=0 )
    {
      --mLevel;
    }
  }

  static void decLevel()
  {
    if(mLevel != LED_LEVEL_MAX)
    {
      ++mLevel;
    }
  }

  static Color getLevel()
  {
    return mColor >> mLevel;
  }

private:
  static uint8_t mCounter;
  static Color mColor;
  static uint8_t mLevel; // brightness factor
};