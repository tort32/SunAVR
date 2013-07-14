#include "stdafx.h"
#include "IR.h"

//#define TEST_ON_LCD 1

#ifdef TEST_ON_LCD
#include "LCD.h"
#endif

// Signal capturing code placed here cause it is hidden from the user code
class IR_SignalCapture
{
private:
  enum {
    STATE_IDLE = 0x0,
    STATE_START = 0x1,

    STATE_CMD_MASK = 0x20, // 6th bit
    STATE_DATA_MASK = 0x10, // 5th bit
    STATE_BIT_MASK = 0x0F, // bits counter mask (0..15), 0..7 - data byte, 8..15 - inverted byte
    STATE_BIT_INV = 0x08, // 8th bit = begin of inverted byte

    STATE_CMD_ADR = STATE_CMD_MASK, // 0x20
    STATE_CMD_ADR_INV = STATE_CMD_ADR | STATE_BIT_INV, // 0x28
    STATE_CMD_DATA = STATE_CMD_MASK | STATE_DATA_MASK, // 0x30
    STATE_CMD_DATA_INV = STATE_CMD_DATA | STATE_BIT_INV, // 0x38
    STATE_STOP = 0x40,

    STATE_ERROR = 0x80,
  };
  enum {
    TCNT_LOW = 10, // threshold = 10
    // '0' = 18 ticks
    TCNT_ZERO = 20, // threshold = 20
    // '1' = 35 ticks
    TCNT_ONE = 40, // threshold = 40
    TCNT_REPEAT = 40,
    // Repeat = 43 ticks
    TCNT_START = 60, // threshold = 60
    // Start = 78 ticks
  };
public:
  inline static void init()
  {
    // Configure 8-bit Counter0 for the time measurement
    TIMSK |= _BV(TOIE0); // enable Counter0 Overflow

    // Configure INT input
    MCUCR = _BV(ISC11) | _BV(ISC10); // generate INT1 on rising edge
    setbits(GICR, _BV(INT1)); // enable INT1

    mState = STATE_IDLE;
  }

  static void startCounter()
  {
    TCNT0 = 0x00; // init counter
    TCCR0 = _BV(CS01) | _BV(CS00); // set prescaler to 64
  }

  static void stopCounter()
  {
    TCCR0 = 0x00; // Stop counter
  }

  inline static void stopSignalCapture()
  {
    // Failed to catch the sequence
    stopCounter();
#ifdef TEST_ON_LCD
    if(mState != STATE_IDLE)
    {
      LCD::print('L');
    }
#endif
    mState = STATE_IDLE;
  }

  inline static void onSignalCapture()
  {
    uint8_t cnt = TCNT0;
    if(mState == STATE_IDLE)
    {
      // First pulse, begin capturing
      startCounter();
      mState = STATE_START;
    }
    else if(mState == STATE_START)
    {
      if(cnt > TCNT_START)
      {
        // BEGIN
        startCounter();
        mState = STATE_CMD_ADR;
      }
      else if(cnt > TCNT_REPEAT)
      {
        // REPEAT
        stopCounter();
        mState = STATE_IDLE;

        // TODO: call handler again
#ifdef TEST_ON_LCD
        LCD::print('R');
#else
        if(IR::mCallOnRepeat && IR::mpCommandCallback != NULL)
        {
          IR::mpCommandCallback(mData[0], mData[1]);
        }
#endif
      }
      else
      {
        // Signal too short
        mState = STATE_ERROR;
#ifdef TEST_ON_LCD
        LCD::print('!');
        LCD::printDigit3(cnt);
#endif
      }
    }
    else if(mState & STATE_CMD_MASK)
    {
      startCounter();

      uint8_t bit = (mState & STATE_BIT_MASK); // bit = 0..15
      uint8_t dataIdx = (mState & STATE_DATA_MASK) >> 4; // 0 - if state in STATE_CMD_ADR, 1 - if state in STATE_CMD_DATA

      if(cnt < TCNT_ZERO)
      {
        // '0'
        if(bit < STATE_BIT_INV)
        {
          // Set data bit by 0
          clrbits(mData[dataIdx], 1 << bit);
        }
        else
        {
          // Check inverted bit
          if(getbits(mData[dataIdx], 1 << (bit - STATE_BIT_INV)) == 0)
          {
            mState = STATE_ERROR;
#ifdef TEST_ON_LCD
            LCD::print('^');
#endif
          }
        }
        ++mState; // move to the next bit
      }
      else if(cnt < TCNT_ONE)
      {
        // '1'
        if(bit < STATE_BIT_INV)
        {
          // Set data bit by 1
          setbits(mData[dataIdx], 1 << bit);
        }
        else
        {
          // TODO: check inverted bit
          if(getbits(mData[dataIdx], 1 << (bit - STATE_BIT_INV)) != 0)
          {
            mState = STATE_ERROR;
#ifdef TEST_ON_LCD
            LCD::print('^');
#endif
          }
        }
        ++mState; // move to the next bit
      }
      else
      {
        // Signal too long
        mState = STATE_ERROR;
#ifdef TEST_ON_LCD
        LCD::print('?');
        LCD::printDigit3(cnt);
#endif
      }

      if(mState == STATE_STOP)
      {
        // Command received
        stopCounter();
        mState = STATE_IDLE;

        // Use command data
#ifdef TEST_ON_LCD
        LCD::printDigit2(mData[0], LCD::HEX);
        LCD::printDigit2(mData[1], LCD::HEX);
#else
        if(IR::mpCommandCallback != NULL)
        {
          IR::mpCommandCallback(mData[0],mData[1]);
        }
#endif
      }
    }
  }
private:
  static volatile uint8_t mState;
  static volatile uint8_t mData[2]; // Address, Command
};

//////////////////////////////////////////////////////////////////////////

volatile uint8_t IR_SignalCapture::mState;
volatile uint8_t IR_SignalCapture::mData[2];
IR::CommandCallback IR::mpCommandCallback;
bool IR::mCallOnRepeat;

void IR::init(CommandCallback pFunc, bool callOnRepeat)
{
  mpCommandCallback = pFunc;
  mCallOnRepeat = callOnRepeat;
  IR_SignalCapture::init();
}

//////////////////////////////////////////////////////////////////////////

ISR(INT1_vect)
{
  cli();
  IR_SignalCapture::onSignalCapture();
  sei();
}

ISR(TIMER0_OVF_vect)
{
  cli();
  IR_SignalCapture::stopSignalCapture();
  sei();
}
