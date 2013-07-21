#pragma once
#include "stdafx.h"

namespace SYSTEM
{
  void init();
  inline void update()
  {
    uint8_t cnt = LED::update();
    if(cnt == 0)
    {
      extern void checkCmd();
      checkCmd();

      /*extern uint8_t mUpdateCnt;
      ++mUpdateCnt;
      if(mUpdateCnt == 0x00)
      {
        extern void UpdateCycle();
        UpdateCycle();
        //mUpdateCnt = 0;
      }*/
    }
  }
}
