#pragma once

// Device frequency
// Should be in consistence with Makefile constant
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stddef.h>
#include "binary.h"
#include "customfont.h"

// Helper macros
#define setbits(port,mask)	(port)|=(mask)
#define clrbits(port,mask)	(port)&=~(mask)
#define tglbits(port,mask)	(port)^=(mask)
#define wrtbits(port,bits,mask) (port)=((port)&(~(mask)))|((bits)&(mask))

#define getbits(port,mask) ((port)&(mask))
#define is_bits(port,mask) (((port)&(mask))!=0)

#define HI(wd) ((BYTE)((0xff00&wd)>>8))
#define LO(wd) ((BYTE)(0x00ff&wd))

#define CHK_BUT(pin,mask) ((~(pin))&(mask))

enum
{
  ERROR = 0,
  SUCCESS = 1,

  ERROR0 = 0xE0,
  ERROR1 = 0xE1,
  ERROR2 = 0xE2,
  ERROR3 = 0xE3,
  ERROR4 = 0xE4,
  ERROR5 = 0xE5,
  ERROR6 = 0xE6,
  ERROR7 = 0xE7,
  ERROR8 = 0xE8,
  ERROR9 = 0xE9,
};

// byte data inc/dec without overflow
inline void incByte(uint8_t& data, uint8_t max = 255)
{
  if(data != max) ++data;
}

inline void decByte(uint8_t& data, uint8_t min = 0)
{
  if(data != min) --data;
}

inline uint16_t attenuate(uint16_t data, uint8_t semilevels)
{
  uint8_t level = (semilevels >> 1);
  uint16_t val = data >> level;
  if(semilevels % 2)
    val -= data >> (level + 2); // -25%
  return val;
}
