#pragma once
#include "stdafx.h"
#include "twi.h"

#define EEPROM_TWI_ID B1101000  // 24C02 slave 7bit address

class EEPROM
{
  uint8_t ReadPage(uint8_t page, uint8_t *data)
  {
    //calculate page address
    uint8_t addr = 0;
    uint8_t i;
    addr = page<<4;
    TWI::Start();
    if (TWI::GetStatus() != 0x08)
      return ERROR;
    //select page start address and send A2 A1 A0 bits send write command
    TWI::Write((EEPROM_TWI_ID|(addr>>3))&(~1));
    if (TWI::GetStatus() != 0x18)
      return ERROR;
    //send the rest of address
    TWI::Write((addr<<4));
    if (TWI::GetStatus() != 0x28)
      return ERROR;
    //send start
    TWI::Start();
    if (TWI::GetStatus() != 0x10)
      return ERROR;
    //select devise and send read bit
    TWI::Write(((EEPROM_TWI_ID)|(addr>>3))|1);
    if (TWI::GetStatus() != 0x40)
      return ERROR;
    for (i=0; i<15; i++)
    {
      *data++ = TWI::ReadACK();
      if (TWI::GetStatus() != 0x50)
        return ERROR;
    }  
    *data = TWI::ReadNACK();
    if (TWI::GetStatus() != 0x58)
      return ERROR;
    TWI::Stop();
    return SUCCESS;
  }
};