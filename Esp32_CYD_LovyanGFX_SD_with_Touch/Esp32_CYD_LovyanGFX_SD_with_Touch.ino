/*
  Device: ESP32 Cheap Yellow Display (CYD) with 2.8-inches
          TFT display with Resistive Touch Controller XPT2046.
          As this devices comes in two variants be sure to 
          define the right one by setting the 
          #define DISPLAY_CYD_2USB to:
          false // if it is a device with one USB (Micro) connector (ILI9341 driver)
          true // if the device has two (Micro- and C) USB connectors (ST7789 driver)

  This sketch is loading JPG images from an inserted SD-Card and
  displays them on the display. The next image is loaded after a
  touch on the screen. The (raw) positions of the touch are 
  displayed and the next image is loaded and displayed.

  This is using the AUTODETECT feature of LovyanGFX.
  As this sketch is for an ESP32 Cheap Yellow Display please select
  'ESP32-2432S028R CYD' as your board !
  If using another board you probably will receive an error like 
  'Compilation error: 'CYD_TFT_SCK' was not declared in this scope'

  Please insert an SD-Card that has two JPG files in the root folder:
  Wave_240_320.jpg
  GrandPrismaticSpring_240_320.jpg

  The GPIO and SPI Bus connections are done based on the work by
  'embedded-kiddie', see: 
  https://github.com/embedded-kiddie/Arduino-CYD-2432S028R
*/

/*
  Environment and Libraries used for this example:
  Arduino 2.3.6 (Windows)
  LovyanGFX 1.2.7
  TJpg_Decoder 1.1.0 (https://github.com/Bodmer/TJpg_Decoder)
*/

// Include the jpeg decoder library
#include <TJpg_Decoder.h>  // https://github.com/Bodmer/TJpg_Decoder

// Include SD
#define FS_NO_GLOBALS
#include <FS.h>
/*
#ifdef ESP32
  #include "SPIFFS.h" // ESP32 only
#endif
*/
#define SD_CS 5

// LGFX Autodetect
// Don't forget to select 'ESP32-2432S028R CYD'
#if USE_AUTODETECT
// D-duino-32 XS, WT32-SC01, and PyBadge are not supported since the panel ID cannot be read.
#define LGFX_AUTODETECT
#include <LovyanGFX.h>
#else
// false: Panel driver: ILI9341 (micro-USB x 1 type)
// true : Panel driver: ST7789  (micro-USB x 1 + USB-C x 1 type)
#define DISPLAY_CYD_2USB false
#include "LGFX_ESP32_2432S028R_CYD.h"
#endif  // USE_AUTODETECT

LGFX tft;

const uint8_t MAX_IMAGES = 2;
//String fileNames[MAX_IMAGES] = { "/panda.jpg", "/GrandPrismaticSpring_240_320_ys08_0398_P1020005.jpg" };
String fileNames[MAX_IMAGES] = { "/Wave_240_320.jpg", "/GrandPrismaticSpring_240_320.jpg" };
uint8_t imageIndex = 0;

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Stop further decoding as image is running off bottom of screen
  if (y >= tft.height()) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n Testing TJpg_Decoder library");

  // Initialise SD before TFT
  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD.begin failed!"));
    while (1) delay(0);
  }
  Serial.println("\r\nInitialisation done.");

  // Initialise the TFT
  tft.begin();
  //tft.setRotation(2); // it is already in portrait orientation
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);  // We need to swap the colour bytes (endianess)

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  tft.drawCentreString("Touch Image -> next", tft.width() / 2, 20, 2);
  tft.drawCentreString("Touch Image -> next", tft.width() / 2, 290, 2);
}

void displayImage(String imagename) {
  Serial.printf("displayImage filename: %s\n", imagename);
  //tft.fillScreen(TFT_WHITE);
  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0;
  TJpgDec.getSdJpgSize(&w, &h, imagename);
  Serial.print("Width = ");
  Serial.print(w);
  Serial.print(", height = ");
  Serial.println(h);

  // Draw the image, top left at 0,0
  TJpgDec.drawSdJpg(0, 0, imagename);

  //displayImage(fileNames[imageIndex]);
  tft.drawCentreString("Touch Image -> next", tft.width() / 2, 20, 2);
  tft.drawCentreString("Touch Image -> next", tft.width() / 2, 290, 2);
}

void loop() {
  int32_t x, y;
  lgfx::v1::touch_point_t tp;
  if (tft.getTouch(&x, &y)) {
    char buf[40];
    sprintf(buf, "Touch at rawX: %d rawY: %d", x, y);
    Serial.println(buf);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    sprintf(buf, "Touch at rawX: %d", x);
    tft.drawCentreString(buf, tft.width() / 2, tft.height() / 2 - 20, 2); // the touch point
    sprintf(buf, "Touch at rawY: %d", y);
    tft.drawCentreString(buf, tft.width() / 2, tft.height() / 2, 2); // the touch point
    delay(1000);
    imageIndex++;
    if (imageIndex > (MAX_IMAGES - 1)) imageIndex = 0;
    displayImage(fileNames[imageIndex]);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("Touch Image -> next", tft.width() / 2, 20, 2);
    tft.drawCentreString("Touch Image -> next", tft.width() / 2, 290, 2);
  }
}
