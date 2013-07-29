#include "stdafx.h"
#include "LCD.h"
#include "string.h"

// --------- PINS -------------------------------------
//#define USING_RW

#define DATA_DDR  DDRC
#define DATA_PORT PORTC
#define DATA_PINS_MASK 0x0F

// Move nibble data to data pins mask
#define DATA_PINS_LO_NIBBLE(x) ((x & 0x0F))
#define DATA_PINS_HI_NIBBLE(x) ((x & 0xF0) >> 4)

#define CONTROL_DDR  DDRB
#define CONTROL_PORT PORTB
#define CONTROL_RS _BV(PB0)
#define CONTROL_EN _BV(PB1)
#define CONTROL_BL _BV(PB3)
//#define CONTROL_RW _BV(PB4)

#ifdef LCD_BUFFERED_INPUT
#include "FIFO.h"
#endif

//--------------------------------------------------------

namespace LCD {

  const uint8_t mLines = 2; // 1 or 2 lines

#ifdef LCD_BUFFERED_INPUT
  struct BufferData
  {
    enum Type {
      COMMAND,
      CHAR,
    } mType;
    uint8_t mData;
    BufferData() {};
    BufferData(Type type, uint8_t data): mType(type), mData(data) {};
    void set(Type type, uint8_t data)
    {
       mType = type;
       mData = data;
    }
  };

  FIFO<BufferData,64> mBuffer;
#endif

  //pulse the Enable pin high (for a microsecond).
  //This clocks whatever command or data is in DB4~7 into the LCD controller.
  void pulseEnablePin(){
    // send a pulse to enable
    setbits(CONTROL_PORT, CONTROL_EN);
    _delay_us(2); // enable pulse must be >1.2us
    clrbits(CONTROL_PORT, CONTROL_EN);
  }

  //push a nibble of data through the the LCD's DB4~7 pins, clocking with the Enable pin.
  //We don't care what RS and RW are, here.
  void pushNibble(uint8_t val){
    wrtbits(DATA_PORT, DATA_PINS_LO_NIBBLE(val), DATA_PINS_MASK);
    pulseEnablePin();
  }

  //push a byte of data through the LCD's DB4~7 pins, in two steps, clocking each with the enable pin.
  void pushByte(uint8_t val){
    wrtbits(DATA_PORT, DATA_PINS_HI_NIBBLE(val), DATA_PINS_MASK);
    pulseEnablePin();
    wrtbits(DATA_PORT, DATA_PINS_LO_NIBBLE(val), DATA_PINS_MASK);
    pulseEnablePin();
  }

  //4bit only, therefore ideally private but may be needed by user
  void commandWriteNibble(uint8_t nibble) {
    clrbits(CONTROL_PORT, CONTROL_RS);
  #ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
  #endif
    pushNibble(nibble);
    _delay_us(40); // commands need > 37us to settle
  }

  void commandWrite(uint8_t val)
  {
    clrbits(CONTROL_PORT, CONTROL_RS);
  #ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
  #endif
    pushByte(val);
    _delay_us(40); // commands need > 37us to settle
  }

#ifdef LCD_BUFFERED_INPUT
  void updateBuffer()
  {
    if(mBuffer.size())
    {
      const BufferData& data = mBuffer.pop_front();
      if(data.mType == BufferData::CHAR)
      {
        setbits(CONTROL_PORT, CONTROL_RS); // data
      }
      else
      {
        clrbits(CONTROL_PORT, CONTROL_RS); // command
      }
#ifdef USING_RW
      clrbits(CONTROL_PORT, CONTROL_RW);
#endif
      pushByte(data.mData);
    }
  }
#endif

  //print the given character at the current cursor position. overwrites, doesn't insert.
  void print(uint8_t val)
  {
#ifdef LCD_BUFFERED_INPUT
    mBuffer.push_back().set(BufferData::CHAR, val);
#else
    //set the RS and RW pins to show we're writing data
    setbits(CONTROL_PORT, CONTROL_RS);
  #ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
  #endif
    //let pushByte worry about the intricacies of Enable, nibble order.
    pushByte(val);
    _delay_us(40); // commands need > 37us to settle
#endif
  }

  //print the given string to the LCD at the current cursor position.  overwrites, doesn't insert.
  //While I don't understand why this was named printIn (PRINT IN?) in the original LiquidCrystal library, I've preserved it here to maintain the interchangeability of the two libraries.
  void printIn(const char* msg)
  {
#ifdef LCD_BUFFERED_INPUT
    for(uint8_t i = 0; msg[i] != 0; i++)
    {
      mBuffer.push_back().set(BufferData::CHAR, msg[i]);
    }
#else
    setbits(CONTROL_PORT, CONTROL_RS);
  #ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
  #endif
    for(uint8_t i = 0; msg[i] != 0; i++)
    {
      //let pushByte worry about the intricacies of Enable, nibble order.
      pushByte(msg[i]);
      _delay_us(40); // commands need > 37us to settle
    }
#endif
  }

  void printIn(const char* msg, uint8_t len)
  {
#ifdef LCD_BUFFERED_INPUT
    for(uint8_t i = 0; i < len; i++)
    {
      mBuffer.push_back().set(BufferData::CHAR, msg[i]);
    }
#else
    setbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
#endif
    for(uint8_t i = 0; i < len; i++) {
      pushByte(msg[i]);
      _delay_us(40); // commands need > 37us to settle
    }
#endif
  }

  //send the clear screen command to the LCD
  void clear()
  {
    clrbits(CONTROL_PORT, CONTROL_RS);
  #ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
  #endif
    pushByte(CLEAR);
    _delay_us(1640); // 1.64ms
  }

  void home()
  {
    clrbits(CONTROL_PORT, CONTROL_RS);
  #ifdef USING_RW
    clrbits(CONTROL_PORT, CONTROL_RW);
  #endif
    pushByte(HOME);
    _delay_us(1640); // 1.64ms
  }

  void initBacklight()
  {
    // Use 8-bit Timer2 for backlight PWM
    OCR2 = 128; // set PWM for 50% duty cycle
    TCCR2 |= _BV(WGM21) | _BV(WGM20); // set fast PWM Mode
    TCCR2 |= _BV(COM21); // set none-inverting mode
    TCCR2 |= _BV(CS21) | _BV(CS20); // set prescaler to 32 (~122 Hz)
  }

  void setBacklight(uint8_t level)
  {
    // Set PWM level
    OCR2 = level;
  }

  // initialize LCD after a short pause
  //while there are hard-coded details here of lines, cursor and blink settings, you can override these original settings after calling .init()
  void init()
  {
    // Configure pins as outputs
    {
      setbits(CONTROL_DDR, CONTROL_EN | CONTROL_RS | CONTROL_BL);
  #ifdef USING_RW
      setbits(CONTROL_DDR, CONTROL_RW);
  #endif
      setbits(DATA_DDR, DATA_PINS_MASK);
    }

    initBacklight();

    // LCD init
    {
      // power on. wait 40ms
      _delay_ms(40);

      // 0010
      //commandWriteNibble(0x2); // passing two times to reinit working display after programming the target
      commandWriteNibble(0x2);

      // Function: 0010 NF00 (4-bits,2-lines,5x8-dots)
      commandWrite(FUNCTION | DATA_4_BIT | (mLines == 2 ? TWO_LINES : ONE_LINE) | CHAR_5x8_DOT);

      // Display: 0000 1DCB (Display,Cursor,Blink)
      commandWrite(DISPLAY | DISPLAY_ON | CURSOR_OFF | BLINK_OFF);

      clear();

      // Entry mode: 0000 01IS (Increment,Shift)
      commandWrite(ENTRY | INCREMENT);
    }
  }


  //non-core stuff --------------------------------------
  //move the cursor to the given absolute position.  line numbers start at 1.
  //if this is not a 2-line LCD4Bit_mod instance, will always position on first line.
  void cursorTo(uint8_t line_num, uint8_t x){
    //offset 40 chars in if second line requested
    if (mLines == 2 && line_num == 2){
      x += 0x40;
    }
    uint8_t cmd = SETDDRAM | x;
#ifdef LCD_BUFFERED_INPUT
    mBuffer.push_back().set(BufferData::COMMAND, cmd);
#else
    commandWrite(cmd);
#endif
  }

  //scroll whole display to left
  void leftScroll(uint8_t num_chars, uint8_t delay_time){
    for (uint8_t i=0; i<num_chars; ++i) {
      commandWrite(SHIFT | CURSOR | LEFT);
      _delay_ms(delay_time);
    }
  }

  // Enables LCD, cursor and cursor blinking
  void setDisplay(bool display, bool cursor, bool blink)
  {
    uint8_t cmd = DISPLAY |
      (display ? DISPLAY_ON : DISPLAY_OFF) |
      (cursor ? CURSOR_ON : CURSOR_OFF) |
      (blink ? BLINK_ON : BLINK_OFF);
#ifdef LCD_BUFFERED_INPUT
    mBuffer.push_back().set(BufferData::COMMAND, cmd);
#else
    commandWrite(cmd);
#endif
  }

  // Write char into CGRAM memory
  void buildChar(uint8_t loc, const uint8_t charmap[8]) {
    // we only have 8 locations: 0-7
    commandWrite(SETCGRAM | ((loc&0x7) << 3));
    for(uint8_t i=0; i<8; ++i)
    {
      print(charmap[i]);
    }
    commandWrite(SETDDRAM); // restore writing to DDRAM
  }

  void buildChars(const uint8_t charmap[64]) {
    // update all 8 custom chars
    for(uint8_t loc = 0; loc<8; ++loc)
    {
      commandWrite(SETCGRAM | ((loc&0x7) << 3));
      for(uint8_t i=0; i<8; ++i)
      {
        print(charmap[loc*8 + i]);
      }
    }
    commandWrite(SETDDRAM); // restore writing to DDRAM
  }

  // draw single digit number
  void printDigit(uint8_t value)
  {
    uint8_t ch = value < 10 ? value + '0' : value - 10 + 'A';
    print(ch);
  }

  // draw two digit number
  void printDigit2(uint8_t value, DigitBase base)
  {
    uint8_t d2 = value / base;
    uint8_t d1 = value % base;
    printDigit(d2);
    printDigit(d1);
  }

  // draw three digit number
  void printDigit3(uint16_t value, DigitBase base)
  {
    uint8_t tmp = value / base;
    uint8_t d3 = tmp / base;
    uint8_t d2 = tmp % base;
    uint8_t d1 = value % base;
    printDigit(d3);
    printDigit(d2);
    printDigit(d1);
  }
}
