#pragma once
#include "stdafx.h"
#include "twi.h"

#define EEPROM_TWI_ID B10100000  // 24C02 slave 7bit address

class EEPROM
{
public:
  // Read 16 bytes of memory page
  static uint8_t ReadPage(uint8_t page, uint8_t *data, uint8_t size)
  {
    // calculate page address
    uint16_t addr = page<<3; // page has 8 bytes

    // Start
    TWI::Start();
    if (TWI::GetStatus() != 0x08)
      return ERROR0;

    // Select page start address and send A2 A1 A0 bits send write command
    TWI::Write(EEPROM_TWI_ID);
    if (TWI::GetStatus() != 0x18)
      return ERROR1;

    // write rest of address
    TWI::Write(0xFF & addr);
    if (TWI::GetStatus() != 0x28)
      return ERROR2;

    // Repeated Start
    TWI::Start();
    if (TWI::GetStatus() != 0x10)
      return ERROR3;

    //select devise and send read bit
    TWI::Write(EEPROM_TWI_ID|1);
    if (TWI::GetStatus() != 0x40)
      return ERROR4;

    // Read 16 bytes (15 with ACK and the last one with NACK)
    for (uint8_t i=0; i<size-1; i++)
    {
      *data++ = TWI::ReadACK();
      if (TWI::GetStatus() != 0x50)
        return ERROR5;
    }  
    *data = TWI::ReadNACK();
    if (TWI::GetStatus() != 0x58)
      return ERROR6;

    TWI::Stop();
    return SUCCESS;
  }
};