#pragma once
#include "stdafx.h"

namespace AsyncCall
{
  typedef void (*FunctionCallback)(void);

  enum {
    STATE_IDLE,
    STATE_WORKING,
    STATE_PAUSED,
  };

  void init();

  uint8_t delayTask(uint16_t delayUs, FunctionCallback pCallback);

  // Call to pause task and reuse TIMER0 device
  void pauseTask(FunctionCallback pOverrideFunction = NULL);

  // Call to continue task
  void continueTask();

  uint8_t getState();
}
