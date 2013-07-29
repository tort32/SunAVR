#include "stdafx.h"
#include "AsyncCall.h"

namespace AsyncCall
{
  volatile FunctionCallback mpCallback;
  volatile FunctionCallback mpPausedCallback;
  volatile uint8_t mPrescale;
  volatile uint8_t mCounter;
  volatile uint8_t mState;

  void startTimer()
  {
    TCNT0 = mCounter;
    TCCR0 = mPrescale;
    mState = STATE_WORKING;
  }

  void stopTimer()
  {
    TCCR0 = 0;
    mState = STATE_IDLE;
  }

  void init()
  {
    mpCallback = NULL;
    mPrescale = 0;
    mCounter = 0;

    setbits(TIMSK,_BV(TOIE0)); // enable Counter0 Overflow

    stopTimer();
    mState = STATE_IDLE;
  }

#define F_CPU_M (F_CPU/1000000UL)

  inline void computeTimer(uint16_t delayUs)
  {
    // CS02 CS01 CS00 bits in TCCR0
    // 0    0    0    - stop
    // 0    0    1    - no prescale (1)
    // 0    1    0    - prescale 1/8       F_CPU
    // 0    1    1    - prescale 1/64
    // 1    0    0    - prescale 1/256
    // 1    0    1    - prescale 1/1024

    if(delayUs < 256/F_CPU_M)
    {
      mCounter = (uint8_t)(255 - delayUs * F_CPU_M);
      mPrescale = _BV(CS00); // no prescale
    }
    else if(delayUs < 8192/F_CPU_M)
    {
      mCounter = (uint8_t)(255 - delayUs * F_CPU_M / 64);
      mPrescale = _BV(CS01) | _BV(CS00); // prescaler 1/64
    }
    else
    {
      mCounter = (uint8_t)(255 - delayUs * F_CPU_M / 1024);
      mPrescale = _BV(CS02) | _BV(CS00); // prescaler 1/1024
    }
  }

  uint8_t delayTask(uint16_t delayUs, FunctionCallback pCallback)
  {
    if(mState == STATE_IDLE)
    {
      mpCallback = pCallback;
      computeTimer(delayUs);
      startTimer();
      return SUCCESS;
    }

    if(mState == STATE_WORKING)
    {
      // Current task is not finished yet
      return ERROR;
    }
    else // mState == STATE_PAUSED
    {
      if(mPrescale == 0)
      {
        // Defer the new call
        mpPausedCallback = pCallback;
        computeTimer(delayUs);
        return SUCCESS;
      }
      else
      {
        // Already got deferred call
        return ERROR;
      }
    }
  }

  // Call to pause task and reuse TIMER0 device
  void pauseTask(FunctionCallback pOverrideFunction)
  {
    // Save working state
    mPrescale = TCCR0;
    mCounter = TCNT0;
    stopTimer();

    mpPausedCallback = mpCallback;
    mpCallback = pOverrideFunction;

    mState = STATE_PAUSED;
  }

  // Call to continue task
  void continueTask()
  {
    if(mPrescale == 0)
    {
      TCCR0 = 0;
      mState = STATE_IDLE;
    }
    else
    {
      // Restore working state
      mpCallback = mpPausedCallback;
      startTimer();
    }
  }

  uint8_t getState()
  {
    return mState;
  }
}

ISR(TIMER0_OVF_vect)
{
  cli();

  AsyncCall::stopTimer();

  if(AsyncCall::mpCallback != NULL)
    (*AsyncCall::mpCallback)();

  sei();
}