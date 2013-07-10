#pragma once
#include "stdafx.h"
#include "twi.h"

// Real Time Clock (based on DS1307)

#define DS1307_TWI_ID B1101000  // DS1307 slave 7bit address

// Day of weeks according to DS1307 datasheet
// 1 - Sunday, 2 - Monday, ... 7 - Saturday
const char dayOfWeek[7][3] =
{
  "Su",
  "Mo",
  "Tu",
  "We",
  "Th",
  "Fr",
  "Sa"
};

class RTC
{
public:
  enum
  {
    SEC = 0,
    MIN = 1,
    HOUR = 2,
    DOW = 3,
    DATE = 4,
    MONTH = 5,
    YEAR = 6,
    CONTROL = 7,
    BYTE_MAX = 8,

    SEC_LO = 10,
    SEC_HI = 20,
    MIN_LO = 11,
    MIN_HI = 21,
    HOUR_LO = 12,
    HOUR_HI = 22,
    DATE_LO = 14,
    DATE_HI = 24,
    MONTH_LO = 15,
    MONTH_HI = 25,
    YEAR_LO = 16,
    YEAR_HI = 26,
  };
private:
  enum {
    LO_BCD  = B00001111,
    HI_BCD  = B11110000,

    HI_SEC  = B01110000,
    HI_MIN  = B01110000,
    HI_HR   = B00110000,
    LO_DOW  = B00000111,
    HI_DATE = B00110000,
    HI_MTH  = B00010000,
    HI_YR   = B11110000,
  };
public:
  static uint8_t read()
  {
    TWI::Start();

    TWI::Write((DS1307_TWI_ID << 1)|0); // Write mode

    TWI::Write(0x00); // Select EEPROM address

    TWI::Start(); // Repeated start

    TWI::Write((DS1307_TWI_ID << 1)|1); // Read mode

    // Read 8 bytes
    uint8_t index = 0;
    for(; index < BYTE_MAX - 1; ++index)
    {
      rtc_bcd[index] = TWI::ReadACK();
    }
    rtc_bcd[index] = TWI::ReadNACK();

    TWI::Stop();

    return 1;
  }

  static uint8_t readByte(uint8_t adr)
  {
    TWI::Start();

    TWI::Write((DS1307_TWI_ID << 1)|0); // Write mode

    TWI::Write(adr); // Select EEPROM address

    TWI::Start(); // Repeated start

    TWI::Write((DS1307_TWI_ID << 1)|1); // Read mode

    uint8_t data = TWI::ReadNACK(); // Read one byte

    TWI::Stop();

    return data;
  }

  static uint8_t write()
  {
    TWI::Start();

    TWI::Write((DS1307_TWI_ID << 1)|0); // Write mode

    TWI::Write(0x00); // Set address

    for(uint8_t i=0; i<BYTE_MAX; ++i)
    {
      TWI::Write(rtc_bcd[i]);
    }

    TWI::Stop();

    return 1;
  }

  static uint8_t get(uint8_t c) // acquire individual RTC item from buffer, return as byte
  {
    switch(c)
    {
    case SEC:
      return get(SEC_HI) * 10 + get(SEC_LO);
    case MIN:
      return get(MIN_HI) * 10 + get(MIN_LO);
    case HOUR:
      return get(HOUR_HI) * 10 + get(HOUR_LO);
    case DOW:
      return (rtc_bcd[DOW] & LO_DOW);
    case DATE:
      return get(DATE_HI) * 10 + get(DATE_LO);
    case MONTH:
      return get(MONTH_HI) * 10 + get(MONTH_LO);
    case YEAR:
      return get(YEAR_HI) * 10 + get(YEAR_LO);

    case SEC_LO:
      return (rtc_bcd[SEC] & LO_BCD);
    case SEC_HI:
      return (rtc_bcd[SEC] & HI_SEC) >> 4;
    case MIN_LO:
      return (rtc_bcd[MIN] & LO_BCD);
    case MIN_HI:
      return (rtc_bcd[MIN] & HI_MIN) >> 4;
    case HOUR_LO:
      return (rtc_bcd[HOUR] & LO_BCD);
    case HOUR_HI:
      return (rtc_bcd[HOUR] & HI_HR) >> 4;
    case DATE_LO:
      return (rtc_bcd[DATE] & LO_BCD);
    case DATE_HI:
      return (rtc_bcd[DATE] & HI_DATE) >> 4;
    case MONTH_LO:
      return (rtc_bcd[MONTH] & LO_BCD);
    case MONTH_HI:
      return (rtc_bcd[MONTH] & HI_MTH) >> 4;
    case YEAR_LO:
      return (rtc_bcd[YEAR] & LO_BCD);
    case YEAR_HI:
      return (rtc_bcd[YEAR] & HI_BCD) >> 4;
    case CONTROL:
      return rtc_bcd[CONTROL];
    } // end switch
    return 0;
  }

  static void set(uint8_t c, uint8_t v)
  {
    switch(c)
    {
    case SEC:
      wrtbits(rtc_bcd[SEC], getNibbles(v), HI_SEC | LO_BCD); break;
    case MIN:
      wrtbits(rtc_bcd[MIN], getNibbles(v), HI_MIN | LO_BCD); break;
    case HOUR:
      // TODO : AM/PM  12HR/24HR
      wrtbits(rtc_bcd[HOUR], getNibbles(v), HI_HR | LO_BCD); break;
    case DOW:
      wrtbits(rtc_bcd[DOW], v, LO_DOW); break;
    case DATE:
      wrtbits(rtc_bcd[DATE], getNibbles(v), HI_DATE | LO_BCD); break;
    case MONTH:
      wrtbits(rtc_bcd[MONTH], getNibbles(v), HI_MTH | LO_BCD); break;
    case YEAR:
      rtc_bcd[YEAR] = getNibbles(v); break;

    case SEC_HI:
      wrtbits(rtc_bcd[SEC], v << 4, HI_SEC); break;
    case SEC_LO:
      wrtbits(rtc_bcd[SEC], v, LO_BCD); break;
    case MIN_HI:
      wrtbits(rtc_bcd[MIN], v << 4, HI_MIN); break;
    case MIN_LO:
      wrtbits(rtc_bcd[MIN], v, LO_BCD); break;
    case HOUR_HI:
      wrtbits(rtc_bcd[HOUR], v << 4, HI_HR); break;
    case HOUR_LO:
      wrtbits(rtc_bcd[HOUR], v, LO_BCD); break;

    case DATE_HI:
      wrtbits(rtc_bcd[DATE], v << 4, HI_DATE); break;
    case DATE_LO:
      wrtbits(rtc_bcd[DATE], v, LO_BCD); break;
    case MONTH_HI:
      wrtbits(rtc_bcd[MONTH], v << 4, HI_MTH); break;
    case MONTH_LO:
      wrtbits(rtc_bcd[MONTH], v, LO_BCD); break;
    case YEAR_HI:
      wrtbits(rtc_bcd[YEAR], v << 4, HI_BCD); break;
    case YEAR_LO:
      wrtbits(rtc_bcd[YEAR], v, LO_BCD); break;
    } // end switch
  }
private:
  static uint8_t getNibbles(uint8_t v)
  {
    // tens go in HI nibble
    // units go in LO nibble
    return ((v / 10) << 4) + (v % 10);
  }
private:
  static uint8_t rtc_bcd[8];
};
