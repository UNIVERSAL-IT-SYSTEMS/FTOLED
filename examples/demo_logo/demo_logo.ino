/*
 * Display the Freetronics logo and full colour cube, same as the packaging sticker.
 *
 * This sketch works by loading the file "Label.bmp" from the SD card, so you'll need
 * to copy that file to the SD card before running it.
 *
 */
#include <SPI.h>
#include <SD.h>
#include <FTOLED.h>

const byte pin_cs = 7;
const byte pin_dc = 2;
const byte pin_reset = 3;

const byte pin_sd_cs = 4;

OLED oled(pin_cs, pin_dc, pin_reset);

#ifdef __AVR__
// On AVR, error messages are stored in PROGMEM so they don't take up RAM
#define MSG_NOSD F("MicroSD card not found")
#define MSG_FILENOTFOUND F("Label.bmp not found")
#define MSG_BMPFAIL F("Failed to load BMP: ")
#else
#define MSG_NOSD "MicroSD card not found"
#define MSG_FILENOTFOUND "Label.bmp not found"
#define MSG_BMPFAIL "Failed to load BMP: "
#endif

// Text box is used to display error message if something
// fails to load
OLED_TextBox text(oled);

void setup()
{
  Serial.begin(115200);
  while(!SD.begin(pin_sd_cs)) {
    Serial.println(MSG_NOSD);
    text.println(MSG_NOSD);
    delay(500);
  }
  oled.begin();

  File image = SD.open("Label.bmp");
  if(!image) {
    text.println(MSG_FILENOTFOUND);
    Serial.println(MSG_FILENOTFOUND);
  } else {
    int r = (int) oled.displayBMP(image, 0, 0);
    if(r) {
      Serial.print(MSG_BMPFAIL);
      Serial.println(r);
      text.print(MSG_BMPFAIL);
      text.println(r);
    }
  }
}

void loop()
{
}
