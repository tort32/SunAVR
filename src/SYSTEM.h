#pragma once
#include "stdafx.h"

namespace SYSTEM
{
  void init();
  inline void update()
  {
    uint8_t cnt = LED::update();
    if(cnt == 0) // ~300Hz
    {
      extern void checkCmd();
      checkCmd();

      extern void UpdateAnim();
      UpdateAnim();
    }
  }
}
