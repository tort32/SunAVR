#include "stdafx.h"
#include "LCD.h"
#include "twi.h"
#include "RTC.h"
#include "IR.h"

void testLCD()
{
  char string1[]={'H','E','L','L','O',' ','W','O','R','L','D',' ','1','2','3','4'};
  char string2[]={0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF};

  LCD::printIn(string1, 16);
  LCD::cursorTo(2, 0);
  LCD::printIn(string2, 16);

  // clear display
  _delay_ms(1000);
  LCD::clear();

  LCD::setDisplay(false, false, false);

  // test code for custom characters
  LCD::buildChars(customChars);

  LCD::cursorTo(1,0);
  for(uint8_t i = 0; i < 16; ++i)
    LCD::print(i);

  LCD::cursorTo(2,0);
  LCD::printIn("0123456789ABCDEF");

  LCD::cursorTo(1,0);
}

void setTime()
{
  // Set time

  RTC::read();

  RTC::set(RTC::YEAR, 13);
  RTC::set(RTC::MONTH, 07);
  RTC::set(RTC::DATE, 07);

  RTC::set(RTC::HOUR, 2);
  RTC::set(RTC::MIN, 56);
  RTC::set(RTC::SEC, 00);

  RTC::set(RTC::DOW, 1);

  RTC::write();
}
/* ------------------------------------------------------------------------- */
void init(void)
{
  LCD::init();
  //IR::init();
  TWI::Init();
  sei();
}

void playBacklightLoop()
{
  static bool up = true;
  static uint8_t cnt = 0;
  if(up)
  {
    if(++cnt == 0xFF)
    {
      up = false;
    }
  }
  else
  {
    if(--cnt == 0x00)
    {
      up = true;
    }
  }
  LCD::setBacklight(cnt);
  _delay_ms(10);
}

void showTimeLoop()
{
  RTC::read();

  LCD::cursorTo(1,0);
  LCD::printDigit(RTC::get(RTC::HOUR_HI));
  LCD::printDigit(RTC::get(RTC::HOUR_LO));
  LCD::print(':');
  LCD::printDigit(RTC::get(RTC::MIN_HI));
  LCD::printDigit(RTC::get(RTC::MIN_LO));
  LCD::print(':');
  LCD::printDigit(RTC::get(RTC::SEC_HI));
  LCD::printDigit(RTC::get(RTC::SEC_LO));

  LCD::cursorTo(2,0);

  LCD::printDigit(RTC::get(RTC::DATE_HI));
  LCD::printDigit(RTC::get(RTC::DATE_LO));
  LCD::print('/');
  LCD::printDigit(RTC::get(RTC::MONTH_HI));
  LCD::printDigit(RTC::get(RTC::MONTH_LO));
  LCD::print('/');
  LCD::printDigit(RTC::get(RTC::YEAR_HI));
  LCD::printDigit(RTC::get(RTC::YEAR_LO));

  LCD::print(' ');
  LCD::printIn(dayOfWeek[RTC::get(RTC::DOW)-1],2); // DOW = 1 => Sunday, ... , 7 => Saturday
  //LCD::printDigit(RTC::get(RTC::DOW));

  _delay_ms(200);
}

void loop(void)
{
  showTimeLoop();
}

/* ------------------------------------------------------------------------- */

int main(void)
{
  init();

  while (true)
  {
    loop();
  }

  return 0;
}
