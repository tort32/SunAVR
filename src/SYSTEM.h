#pragma once
#include "stdafx.h"
#include "LCD.h"

namespace SYSTEM
{
  void init();
  inline void update()
  {
    uint16_t cnt = LED::update();
    if(cnt == 0) // ~300Hz
    {
      extern void checkCmd();
      checkCmd();

      extern uint16_t UpdateAnim();
      uint16_t cnt2 = UpdateAnim();
      uint16_t cnt3 = cnt2 % 256; // ~1Hz
      if(cnt3 == 0)
      {
        extern void ReadClock();
        ReadClock();
      } else
      if(cnt3 == 1)
      {
        extern void UpdateClock();
        UpdateClock();
      }
      else
      {
        LCD::updateBuffer();
      }
    }
  }
}
