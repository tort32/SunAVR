#pragma once
#include "stdafx.h"

#define LED_R _BV(PD0)
#define LED_G _BV(PD1)
#define LED_B _BV(PD2)
#define LED_PINS (LED_R|LED_G|LED_B)

#define LED_DDR  DDRD
#define LED_PORT PORTD

#define LED_LEVEL_MAX 14

#define ADD_BITS 2
#define ADD_BITS_LEFT (8 - ADD_BITS)

namespace LED
{
  struct Color
  {
    union
    {
      struct
      {
        uint16_t R;
        uint16_t G;
        uint16_t B;
      };
      uint16_t v[3];
    };
    Color() {};
    Color(uint8_t red, uint8_t green, uint8_t blue): R(red << ADD_BITS), G(green << ADD_BITS), B(blue << ADD_BITS) {};

    // Attenuate color
    // level = 0..15
    inline Color attenuate(uint8_t level) const {
      Color col;
      col.R = ::attenuate(R, level);
      col.G = ::attenuate(G, level);
      col.B = ::attenuate(B, level);
      return col;
    }

    inline Color operator+(const Color& add) const {
      Color col;
      col.R = R + add.R;
      col.G = G + add.G;
      col.B = B + add.B;
      return col;
    }

    // Interpolate between two colors with attenuation
    inline Color interpolate(const Color& col, uint8_t semilevels) const
    {
      return this->attenuate(semilevels) + col.attenuate(15 - semilevels);
    }

    // Interpolate between two colors.
    // semilevels = 0..15 (0 = source color, 16 = destination color)
    Color interpolateLinear(const Color& col, uint8_t semilevels) const
    {
      int16_t deltaR = (int16_t)col.R - (int16_t)R;
      int16_t deltaG = (int16_t)col.G - (int16_t)G;
      int16_t deltaB = (int16_t)col.B - (int16_t)B;

      Color res;
      res.R = (int16_t)R + ((deltaR * semilevels) >> 4);
      res.G = (int16_t)G + ((deltaG * semilevels) >> 4);
      res.B = (int16_t)B + ((deltaB * semilevels) >> 4);
      return res;
    }
  };

  void init();
  void disable();
  void setColor(const Color& color);
  void setLevel(uint8_t level);
  void incLevel();
  void decLevel();
  const Color& getLevel();

  inline uint16_t update()
  {
    extern uint16_t mCounter;
    extern Color mValue;

    if(mCounter == mValue.R) clrbits(LED_PORT, LED_R); // OFF
    if(mCounter == mValue.G) clrbits(LED_PORT, LED_G); // OFF
    if(mCounter == mValue.B) clrbits(LED_PORT, LED_B); // OFF
    if(mCounter == 0 && mValue.R != 0) setbits(LED_PORT, LED_R); // ON
    if(mCounter == 0 && mValue.G != 0) setbits(LED_PORT, LED_G); // ON
    if(mCounter == 0 && mValue.B != 0) setbits(LED_PORT, LED_B); // ON

    if(++mCounter == (256 << ADD_BITS))
    {
      mCounter = 0;
    }

    return mCounter;
  }
}
