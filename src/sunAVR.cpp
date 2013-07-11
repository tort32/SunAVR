#include "stdafx.h"
#include "LCD.h"
#include "twi.h"
#include "RTC.h"
#include "IR.h"
#include "EEPROM.h"

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

enum
{
  CMD_NONE = 0xFF,
  CMD_INIT = 0xFE,

  // LED IR Control Table
  CMD_UP = 0x09,
  CMD_DOWN = 0x1D,
  CMD_OFF = 0x1F,
  CMD_ON = 0x0D,
  CMD_R = 0x19,
  CMD_G = 0x1B,
  CMD_B = 0x11,
  CMD_W = 0x15,
  CMD_01 = 0x17,
  CMD_02 = 0x12,
  CMD_03 = 0x16,
  CMD_FLASH = 0x4D,
  CMD_04 = 0x40,
  CMD_05 = 0x4C,
  CMD_06 = 0x04,
  CMD_STROBE = 0x00,
  CMD_07 = 0x0A,
  CMD_08 = 0x1E,
  CMD_09 = 0x0E,
  CMD_FADE = 0x1A,
  CMD_10 = 0x1C,
  CMD_11 = 0x14,
  CMD_12 = 0x0F,
  CMD_SMOOTH = 0x0C,
};
volatile uint8_t runCmd = CMD_INIT;

void IRCallback(uint8_t adr, uint8_t cmd)
{
  /*LCD::cursorTo(1,0);
  LCD::printIn("ADR=");
  LCD::printDigit2(adr, LCD::HEX);
  LCD::cursorTo(2,0);
  LCD::printIn("CMD=");
  LCD::printDigit2(cmd, LCD::HEX);*/
  runCmd = cmd;
}
/* ------------------------------------------------------------------------- */
void init(void)
{
  LCD::init();
  IR::init(&IRCallback);
  //TWI::Init();
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

void checkCmd()
{
  static uint8_t line = 0x00;
  if(runCmd != CMD_NONE)
  {
    if(runCmd == CMD_UP) // UP
    {
      ++line;
    }
    else if(runCmd == CMD_DOWN) // UP
    {
      --line;
    }

    uint8_t data[8];
    uint8_t result = EEPROM::ReadPage(line, data, 8);
    if(result == SUCCESS)
    {
      LCD::clear();
      LCD::cursorTo(1,0);
      LCD::printIn("PAGE=");
      LCD::printDigit2(line, LCD::HEX);

      /*_delay_ms(1000);
      LCD::cursorTo(1,0);
      for(uint8_t i = 0; i < 8; ++i)
        LCD::printDigit2(data[i], LCD::HEX);*/

      LCD::cursorTo(2,0);
      //for(uint8_t i = 8; i < 16; ++i)
      for(uint8_t i = 0; i < 8; ++i)
        LCD::printDigit2(data[i], LCD::HEX);
    }
    else
    {
      LCD::clear();
      LCD::cursorTo(1,0);

      LCD::printIn("PAGE=");
      LCD::printDigit2(line, LCD::HEX);

      LCD::cursorTo(2,0);
      LCD::printIn("ERROR ");
      LCD::printDigit2(result, LCD::HEX);
    }

    /*LCD::cursorTo(1,0);
    LCD::printDigit2(line, LCD::HEX);*/

    runCmd = CMD_NONE;
  }
  _delay_ms(100);
}

void loop(void)
{
  //showTimeLoop();
  checkCmd();
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
