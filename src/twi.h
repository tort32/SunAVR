#pragma once
#include "stdafx.h"

// I2C bus routine

// TWI SCL frequency
#define F_TWI 10000UL

#if F_CPU/F_TWI < 16
# error "F_TWI is too hight. Should be less than F_CPU/16"
#endif

class TWI
{
public:
  static void Init()
  {
    // SCL_freq = CPU_freq / ( 16 + 2 * TWBR * 4^TWPS )
    // TWBR = (( CPU_freq / SCL_freq ) - 16) / 2; TWPS = 0;

    TWSR = 0x00;
    TWBR = (F_CPU/F_TWI - 16)/2;

    //enable TWI
    TWCR = (1<<TWEN);
  }

  static void Start()
  {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
  }

  static void Stop()
  {
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
  }

  static void Write(uint8_t data)
  {
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
  }

  static uint8_t ReadACK()
  {
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
  }

  //read byte with NACK
  static uint8_t ReadNACK()
  {
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
  }

  static uint8_t GetStatus()
  {
    uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
  }
};
