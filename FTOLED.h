/*

  OLED.h - Support library for Freetronics 128x128 OLED display

  Copyright (C) 2013 Freetronics, Inc. (info <at> freetronics <dot> com)

  Written by Angus Gratton

  Note that this library uses the SPI port for the fastest, low
  overhead writing to the display. Keep an eye on conflicts if there
  are any other devices running from the same SPI port, and that the
  chip select on those devices is correctly set to be inactive when the
  OLED is being written to.

 ---

 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OLED_H_
#define OLED_H_

//Arduino header name is version dependent
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "pins_arduino.h"
#include <SPI.h>
#include <SD.h>
#include "Print.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
// Stub out some common progmem definitions for ARM processors
#define memcpy_P memcpy
static inline uint8_t pgm_read_byte(const void *addr) { return *((uint8_t *)addr); }
#endif


#define ROWS 128
#define COLUMNS 128
#define ROW_MASK (ROWS-1)
#define COLUMN_MASK (COLUMNS-1)

struct Colour
{
  byte red   : 5;
  byte green : 6;
  byte blue  : 5;
};
#define Color Colour

#include "FTOLED_Colours.h"

const byte MAX_RED = 31;
const byte MAX_GREEN = 63;
const byte MAX_BLUE = 31;

// Display mode argument for setDisplayMode()
enum OLED_Display_Mode {
  DISPLAY_OFF = 0,                 // No pixels on
  DISPLAY_ALL_PIXELS_FULL = 1,     // All pixels on GS level 63 (ie max brightness)
  DISPLAY_NORMAL = 2,              // Normal display on
  DISPLAY_INVERSE = 3,
};

// OLED GPIO mode for setGPIO()
enum OLED_GPIO_Mode {
  OLED_HIZ = 0,
  OLED_LOW = 2,
  OLED_HIGH = 3,
};

// Status code returned by displayBMP()
enum BMP_Status {
  BMP_OK = 0,
  BMP_INVALID_FORMAT = 1,
  BMP_UNSUPPORTED_HEADER = 2,
  BMP_TOO_MANY_COLOURS = 3,
  BMP_COMPRESSION_NOT_SUPPORTED = 4,
  BMP_ORIGIN_OUTSIDE_IMAGE = 5
};

class OLED_TextBox;

class OLED
{
  friend class OLED_TextBox;
public:
  OLED(byte pin_ncs, byte pin_dc, byte pin_reset) :
  pin_ncs(pin_ncs),
  pin_dc(pin_dc),
  pin_reset(pin_reset),
  gpio_status(OLED_HIZ | OLED_HIZ<<2)
  {}

  void begin();

  // Set the colour of a single pixel
  void setPixel(const byte x, const byte y, const Colour colour);

  // Fill the screen with a single solid colour
  void fillScreen(const Colour);
  void clearScreen() { fillScreen(BLACK); }

  // Turn the display on or off
  void setDisplayOn(bool on);

  //Draw a line from x1,y1 to x2,y2
  void drawLine( int x1, int y1, int x2, int y2, Colour colour );

  //Draw a box(rectangle) from (x1,y1) to (x2,y2), with sides edgeWidth pixels wide
  void drawBox( int x1, int y1, int x2, int y2, int edgeWidth, Colour colour);

  //Draw a filled box(rectangle) from (x1,y1) to (y1,y2), optionally with sides edgeWidth pixels wide
  void drawFilledBox( int x1, int y1, int x2, int y2, Colour fillColour, int edgeWidth, Colour edgeColour);
  void drawFilledBox( int x1, int y1, int x2, int y2, Colour fillColour) { drawFilledBox(x1,y1,x2,y2,fillColour,0,BLACK); }

  // Draw an outline of a circle of radius r centred at x,y
  void drawCircle( int xCenter, int yCenter, int radius, Colour colour);

  // Draw an filled circle of radius r at x,y centre
  void drawFilledCircle( int xCenter, int yCenter, int radius, Colour fillColour);

  //Select a text font
  void selectFont(const uint8_t* font);

  //Draw a single character
  int drawChar(const int x, const int y, const char letter, const Colour colour, const Colour background);

  //Find the width of a character
  int charWidth(const char letter);

  // Draw a full string
  void drawString(int x, int y, const char *bChars, Colour foreground, Colour background);

  // Bitmap stuff

  // Given 'File' containing a BMP image, show it onscreen with bottom left corner at (x,y)
  BMP_Status displayBMP(File &source, const int x, const int y);
  BMP_Status displayBMP(File &source, const int from_x, const int from_y, const int to_x, const int to_y);

  // Given 'pgm_addr', a pointer to a PROGMEM buffer (or const buffer on ARM) containing a BMP,
  // show it onscreen with bottom left corner at (x,y)
  BMP_Status displayBMP(const uint8_t *pgm_addr, const int x, const int y);
  BMP_Status displayBMP(const uint8_t *pgm_addr, const int from_x, const int from_y, const int to_x, const int to_y);

  /* Set the grayscale table for pixel brightness to one of these precanned defaults */
  void setDefaultGrayscaleTable();
  void setBrightGrayscaleTable();
  void setDimGrayscaleTable();

  /* Set a custom sized grayscale table. "table" must be address of a
     PROGMEM table holding 64 grayscale level values (GS0..GS63), which must be
     strictly incrementing (see section 8.8 in the datasheet.) Values
     in the table can have values 0-180.
  */
  inline void setGrayscaleTable_P(const byte *table);

  void setGPIO1(OLED_GPIO_Mode gpio1);

  /* Set display mode. See enum OLED_Display_Mode, above. */
  inline void setDisplayMode(OLED_Display_Mode mode) {
    assertCS();
    writeCommand(0xA4+(byte)mode);
    releaseCS();
  }

 protected:
  byte pin_ncs;
  byte pin_dc;
  byte pin_reset;
  byte remap_flags;
  byte gpio_status;

  uint8_t *font;

  inline void assertCS() { digitalWrite(pin_ncs, LOW); }
  inline void releaseCS() { digitalWrite(pin_ncs, HIGH); }

  // Note: GPIO0 is panel power on OLED128, hence better to use setDisplayOn()
  void setGPIO0(OLED_GPIO_Mode gpio0);

  /* These protected methods are for implementing basic OLED commands.
     They all assume that the CS is asserted before they've been called
  */
  inline void writeCommand(byte command)
  {
    digitalWrite(pin_dc, LOW);
    SPI.transfer(command);
    digitalWrite(pin_dc, HIGH);
  }

  inline void writeData(byte data)
  {
    SPI.transfer(data);
  }

  inline void writeData(Colour colour)
  {
    writeData((colour.green>>3)|(colour.red<<3));
    writeData((colour.green<<5)|(colour.blue));
  }

  inline void writeCommand(byte command, byte data) {
    writeCommand(command);
    writeData(data);
  }

  inline void writeData(byte *data, unsigned int length)
  {
    for(unsigned int i = 0; i < length; i++) {
      writeData(data[i]);
    }
  }

  inline void setColumn(byte start, byte end) {
    writeCommand(0x15);
    writeData(start);
    writeData(end);
  }

  inline void setRow(byte start, byte end) {
    writeCommand(0x75);
    writeData(start);
    writeData(end);
  }

  inline void setWriteRam() {
    writeCommand(0x5C);
  }

  inline void _setPixel(const byte x, const byte y, const Colour);

  enum OLED_Command_Lock {
    DISPLAY_COMMAND_UNLOCK = 0x12,        // Allow commands (default state)
    DISPLAY_COMMAND_LOCK = 0x16,          // Disallow all commands until/except next UNLOCK

    DISPLAY_COMMAND_LOCK_SPECIAL = 0xB0,  // Lock out "special" commands always (default state)
    DISPLAY_COMMAND_ALLOW_SPECIAL = 0xB1, // Allow "special" commands when unlocked
  };

  // Direct commands to the module
  inline void setCommandLock(OLED_Command_Lock lock_command)
  {
    writeCommand(0xFD, (byte)lock_command);
  }

// Display clock divisor options, used by setDisplayClock
#define DISPLAY_CLOCK_DIV_1 0
#define DISPLAY_CLOCK_DIV_2 1
#define DISPLAY_CLOCK_DIV_4 2
#define DISPLAY_CLOCK_DIV_8 3
#define DISPLAY_CLOCK_DIV_16 4
#define DISPLAY_CLOCK_DIV_32 5
#define DISPLAY_CLOCK_DIV_64 6
#define DISPLAY_CLOCK_DIV_128 7
#define DISPLAY_CLOCK_DIV_256 8
#define DISPLAY_CLOCK_DIV_512 9
#define DISPLAY_CLOCK_DIV_1024 10

  /* set display refresh clock
   * Args:
   * divisor - a constant (DISPLAY_CLOCK_DIV_xxx), see above.
   * frequency - a value 0-15 proportional to oscillator frequency.
   */
  inline void setDisplayClock(byte divisor, byte frequency)
  {
    writeCommand(0xB3, divisor | ((frequency&0x0F)<<4));
  }

  /* set multiplex ratio (Default of ROW_MASK/127 for 128 should be fine in nearly all cases)
   */
  inline void setMultiPlexRatio(byte mux_ratio)
  {
    writeCommand(0xCA, mux_ratio & ROW_MASK);
  }

  /* set display offset row (0-127) */
  inline void setDisplayOffset(byte row)
  {
    writeCommand(0xA2, row & ROW_MASK);
  }

  /* set starting row for display (0-127) */
  inline void setStartRow(byte row)
  {
    writeCommand(0xA1, row & ROW_MASK);
  }

  // Flags for setRemapFormat & setIncrementDirection, defined below
#define REMAP_HORIZONTAL_INCREMENT 0
#define REMAP_VERTICAL_INCREMENT (1<<0)

#define REMAP_COLUMNS_LEFT_TO_RIGHT 0
#define REMAP_COLUMNS_RIGHT_TO_LEFT (1<<1)

#define REMAP_ORDER_BGR 0
#define REMAP_ORDER_RGB (1<<2)

#define REMAP_SCAN_UP_TO_DOWN 0
#define REMAP_SCAN_DOWN_TO_UP (1<<4)

#define REMAP_COM_SPLIT_ODD_EVEN (1<<5)

#define REMAP_COLOR_8BIT 0
#define REMAP_COLOR_RGB565 (1<<6)
#define REMAP_COLOR_18BIT (2<<6)

  /* set up address/pixel remap format, see flags REMAP_xxxx_xxxx above */
  inline void setRemapFormat(byte remap_flags)
  {
    writeCommand(0xA0, remap_flags);
    this->remap_flags = remap_flags;
  }

  /* Set the direction to increment when filling in pixel data (horizontal or vertical)
   *
   * This is the most commonly set part of the 'remap' format, so this function
   * sets it without disturbing the other flags.
   */
  inline void setIncrementDirection(byte direction)
  {
    byte remap = this->remap_flags & ~(REMAP_HORIZONTAL_INCREMENT|REMAP_VERTICAL_INCREMENT);
    remap = remap | (direction & (REMAP_VERTICAL_INCREMENT|REMAP_HORIZONTAL_INCREMENT));
    writeCommand(0xA0, remap);
    this->remap_flags = remap;
  }



  /* set color channel contrasts. A,B,C are R,G,B values unless REMAP_ORDER_BGR is set */
  inline void setColorContrasts(byte a, byte b, byte c)
  {
    writeCommand(0xC1);
    writeData(a);
    writeData(b);
    writeData(c);
  }

  /* Set Master contrast, value 0-15 */
  inline void setMasterContrast(byte contrast)
  {
    writeCommand(0xC7, contrast & 0x0F);
  }

  /* Set the reset (phase 1) and precharge (phase 2) lengths.
     Reset range is 5-31 DCLK periods (odd values only), precharge is 3-15 DCLK periods
   */
  inline void setResetPrechargePeriods(byte resetLength, byte precharge) {
    resetLength = (resetLength&~1)/2 - 1; // Value we write is 3-15 same as the others
    writeCommand(0xB1, (resetLength>>8)|(precharge&0x0F));
  }

  /* Set precharge voltage, level is a proportion of Vcc where 0x00=0.2 0x1F=0.60,
     Default of 0x17 is 0.50 */
  inline void setPrechargeVoltage(byte level) {
    writeCommand(0xBB, level & 0x1F);
  }

  /* Set second precharge period (phase 3) as number of DCLK periods 1-15. Default is 8. */
  inline void setSecondPrechargePeriod(byte clocks) {
    clocks = clocks & 0x0F;
    writeCommand(0xB6, clocks ? clocks : 8);
  }

  /* Set lock bits. Reset means 0x12, 0x16? means nothing works except reset and another unlock? */
  inline void setLockBits(byte lock_bits) {
    writeCommand(0xFD, lock_bits);
  }

  // Internal templated displayBMP method, allows us to treat SD card files and PROGMEM buffers
  // via the same code paths
  template<typename SourceType> BMP_Status _displayBMP(SourceType &source, const int from_x, const int from_y, const int to_x, const int to_y);
};

class OLED_TextBox : public Print {
public:
  OLED_TextBox(OLED &oled, int left = 0, int bottom = 0, int width = COLUMNS, int height = ROWS);
  virtual size_t write(uint8_t);
  void clear();
  void reset();
  void setForegroundColour(Colour colour);
  void setBackgroundColour(Colour colour);
  inline void setForegroundColor(Colour color) { setForegroundColour(color); }
  inline void setBackgroundColor(Colour color) { setBackgroundColour(color); }

private:
  OLED &oled;
  uint8_t left;
  uint8_t bottom;
  uint8_t width;
  uint8_t height;
  int16_t cur_x;
  int16_t cur_y;
  uint8_t max_rows;
  uint16_t buf_sz;
  char *buffer;
  Colour foreground;
  Colour background;
  bool pending_newline;

  void scroll(uint8_t fontHeight);
  void clear_area();
};

// Six byte header at beginning of FontCreator font structure, stored in PROGMEM
struct FontHeader {
  uint16_t size;
  uint8_t fixedWidth;
  uint8_t height;
  uint8_t firstChar;
  uint8_t charCount;
};

#endif
