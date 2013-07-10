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

//--------------------------------------------------------

//pulse the Enable pin high (for a microsecond).
//This clocks whatever command or data is in DB4~7 into the LCD controller.
void LCD::pulseEnablePin(){
  // send a pulse to enable
  setbits(CONTROL_PORT, CONTROL_EN);
  _delay_ms(1); // enable pulse must be >1.2us
  clrbits(CONTROL_PORT, CONTROL_EN);
}

//push a nibble of data through the the LCD's DB4~7 pins, clocking with the Enable pin.
//We don't care what RS and RW are, here.
void LCD::pushNibble(uint8_t value){
  wrtbits(DATA_PORT, DATA_PINS_LO_NIBBLE(value), DATA_PINS_MASK);
  pulseEnablePin();
}

//push a byte of data through the LCD's DB4~7 pins, in two steps, clocking each with the enable pin.
void LCD::pushByte(uint8_t value){
  wrtbits(DATA_PORT, DATA_PINS_HI_NIBBLE(value), DATA_PINS_MASK);
  pulseEnablePin();
  wrtbits(DATA_PORT, DATA_PINS_LO_NIBBLE(value), DATA_PINS_MASK);
  pulseEnablePin();
}


//stuff the library user might call---------------------------------

void LCD::commandWriteNibble(uint8_t nibble) {
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushNibble(nibble);
  _delay_us(40); // commands need > 37us to settle
}


void LCD::commandWrite(uint8_t value) {
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushByte(value);
  _delay_us(40); // commands need > 37us to settle
}


//print the given character at the current cursor position. overwrites, doesn't insert.
void LCD::print(uint8_t value) {
  //set the RS and RW pins to show we're writing data
  setbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  //let pushByte worry about the intricacies of Enable, nibble order.
  pushByte(value);
  _delay_us(40); // commands need > 37us to settle
}


//print the given string to the LCD at the current cursor position.  overwrites, doesn't insert.
//While I don't understand why this was named printIn (PRINT IN?) in the original LiquidCrystal library, I've preserved it here to maintain the interchangeability of the two libraries.
void LCD::printIn(const char* msg) {
  setbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  uint8_t i;  //fancy int.  avoids compiler warning when comparing i with strlen()'s uint8_t
  uint8_t len = strlen(msg);
  for (i=0; i < len; i++){
    //let pushByte worry about the intricacies of Enable, nibble order.
    pushByte(msg[i]);
    _delay_us(40); // commands need > 37us to settle
  }
}

void LCD::printIn(const char* msg, uint8_t len) 
{
  setbits(CONTROL_PORT, CONTROL_RS);
  for (uint8_t i=0;i < len ;i++) {
    pushByte(msg[i]);
    _delay_us(40); // commands need > 37us to settle
  }
}


//send the clear screen command to the LCD
void LCD::clear(){
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushByte(CLEAR);
  _delay_us(1640); // 1.64ms
}

void LCD::home(){
  clrbits(CONTROL_PORT, CONTROL_RS);
#ifdef USING_RW
  clrbits(CONTROL_PORT, CONTROL_RW);
#endif
  pushByte(HOME);
  _delay_us(1640); // 1.64ms
}

void LCD::initBacklight()
{
  // Use 8-bit Timer2 for backlight PWM
  OCR2 = 128; // set PWM for 50% duty cycle
  TCCR2 |= _BV(WGM21) | _BV(WGM20); // set fast PWM Mode
  TCCR2 |= _BV(COM21); // set none-inverting mode
  TCCR2 |= _BV(CS21) | _BV(CS20); // set prescaler to 32 (~122 Hz)
}

void LCD::setBacklight(uint8_t level)
{
  // Set PWM level
  OCR2 = level;
}

// initialize LCD after a short pause
//while there are hard-coded details here of lines, cursor and blink settings, you can override these original settings after calling .init()
void LCD::init()
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
void LCD::cursorTo(uint8_t line_num, uint8_t x){
  //offset 40 chars in if second line requested
  if (mLines == 2 && line_num == 2){
    x += 0x40;
  }
  commandWrite(SETDDRAM | x);
}

//scroll whole display to left
void LCD::leftScroll(uint8_t num_chars, uint8_t delay_time){
  for (uint8_t i=0; i<num_chars; ++i) {
    commandWrite(SHIFT | CURSOR | LEFT);
    _delay_ms(delay_time);
  }
}

// Enables LCD, cursor and cursor blinking
void LCD::setDisplay(bool display, bool cursor, bool blink)
{
  commandWrite(DISPLAY |
    (display ? DISPLAY_ON : DISPLAY_OFF) |
    (cursor ? CURSOR_ON : CURSOR_OFF) |
    (blink ? BLINK_ON : BLINK_OFF) );
}

// Write char into CGRAM memory
void LCD::buildChar(uint8_t loc, const uint8_t charmap[8]) {
  // we only have 8 locations: 0-7
  commandWrite(SETCGRAM | ((loc&0x7) << 3));
  for(uint8_t i=0; i<8; ++i)
  {
    print(charmap[i]);
  }
  commandWrite(SETDDRAM); // restore writing to DDRAM
}

void LCD::buildChars(const uint8_t charmap[64]) {
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
void LCD::printDigit(uint8_t value)
{
  uint8_t ch = value < 10 ? value + '0' : value - 10 + 'A';
  print(ch);
}

// draw two digit number
void LCD::printDigit2(uint8_t value, DigitBase base)
{
  uint8_t d2 = value / base;
  uint8_t d1 = value % base;
  printDigit(d2);
  printDigit(d1);
}

// draw three digit number
void LCD::printDigit3(uint8_t value, DigitBase base)
{
  uint8_t tmp = value / base;
  uint8_t d3 = tmp / base;
  uint8_t d2 = tmp % base;
  uint8_t d1 = value % base;
  printDigit(d3);
  printDigit(d2);
  printDigit(d1);
}
