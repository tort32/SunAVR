#include "stdafx.h"
#include "RGBLED.h"

namespace LED
{
  uint16_t mCounter;
  Color mColor;
  Color mValue;
  uint8_t mLevel; // brightness factor

  void updatePrescalers();

  void init()
  {
    mColor = Color(0xFF,0xFF,0xFF);
    mLevel = 4;
    mCounter = 0;
    updatePrescalers();

    setbits(LED_DDR, LED_PINS); // output
  }

  void disable()
  {
    // SHUT ALL OFF
    clrbits(LED_PORT, LED_PINS);
  }

  void setColor(const Color& color)
  {
    mColor = color;
    updatePrescalers();
  }

  void setLevel(uint8_t level)
  {
    mLevel = level;
    updatePrescalers();
  }

  void incLevel()
  {
    if(mLevel !=0 )
    {
      --mLevel;
      updatePrescalers();
    }
  }

  void decLevel()
  {
    if(mLevel != LED_LEVEL_MAX)
    {
      ++mLevel;
      updatePrescalers();
    }
  }

  const Color& getLevel()
  {
    return mValue;
  }

  void updatePrescalers()
  {
    mValue = mColor.attenuate(mLevel);
  }
}