#include "stdafx.h"
#include "RGBLED.h"

namespace LED
{
  uint8_t mCounter;
  Color mColor;
  Color mValue;
#ifdef LED_PRECISE
  Color mPreScl;
  Color mPreSclCnt;
#endif
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

  Color getLevel()
  {
    return mValue;
  }

  Color getPreScl()
  {
#ifdef LED_PRECISE
    return mPreScl;
#else
    return Color(0,0,0);
#endif
  }

  void updatePrescalers()
  {
#ifdef LED_PRECISE
    for(uint8_t i = 0; i < 3; ++i)
    {
      uint8_t val = mColor.v[i] >> mLevel;
      if(val >= 0x10)
      {
        mValue.v[i]= val;
        mPreScl.v[i] = 1;
      }
      else if(val >= 0x08)
      {
        mValue.v[i] = mColor.v[i] >> (mLevel - 2);
        mPreScl.v[i] = 2;
      }
      else/* if(val >= 0x04)*/
      {
        mValue.v[i] = mColor.v[i] >> (mLevel - 4);
        mPreScl.v[i] = 4;
      }
      /*else
      {
        mValue.v[i] = mColor.v[i] >> (mLevel - 3);
        mPreScl.v[i] = 8;
      }*/
    }
    mPreSclCnt = Color(0,0,0);
#else
    mValue = mColor >> mLevel;
#endif
  }
}