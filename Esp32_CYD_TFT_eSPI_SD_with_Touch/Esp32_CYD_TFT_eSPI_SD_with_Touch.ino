/*
  Device: ESP32 Cheap Yellow Display (CYD) with 2.8-inches
          TFT display with Resistive Touch Controller.
          As this devices comes in two variants be sure to 
          activate the right one by using the correct
          User Settings file.
          - If it is a device with one USB (Micro) connector
            ILI9341 display use: Setup801_ESP32_CYD_ILI9341_240x320_NoMiso.h
          - If the device has two (Micro- and C) USB connectors
            ST7789 display use:  Setup805_ESP32_CYD_ST7789_240x320_NoMiso.h

  This sketch is loading JPG images from an inserted SD-Card and
  displays them on the display. The next image is loaded after a
  touch on the screen. The (raw) positions of the touch are 
  displayed and the next image is loaded and displayed.

  This is using the TFT_eSPI library for displaying the content and the
  XPT2046 Touchscreen library by Paul Stoffregen to run the Touch
  Controller.

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
  TFT_eSPI Mar 6, 2024
  TJpg_Decoder 1.1.0 (https://github.com/Bodmer/TJpg_Decoder)
*/

// Include the jpeg decoder library
#include <TJpg_Decoder.h>  // https://github.com/Bodmer/TJpg_Decoder

// Include SD
#include "sdcard.h"

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Important: in User_Settings-file leave out the TFT_MISO pin !!!
// User_Settings file in use is 'Setup801_ESP32_CYD_ILI9341_240x320_NoMiso.h'

// This is using the XPT2046 library for XPT2026 Touch
// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen
// Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h> // https://github.com/PaulStoffregen/XPT2046_Touchscreen

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // GPIO23  // T_DIN
#define XPT2046_MISO 39  // GPIO19  // T_OUT
#define XPT2046_CLK 25   // GPIO18   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

const uint8_t DISPLAY_ORIENTATION_PORTRAIT = 0;

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

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 3: touchscreen.setRotation(3);
  touchscreen.setRotation(DISPLAY_ORIENTATION_PORTRAIT);

  // Initialise the TFT
  tft.begin();
  tft.setRotation(DISPLAY_ORIENTATION_PORTRAIT);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);  // We need to swap the colour bytes (endianess)

  sdcard_setup();

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
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z) info on the TFT display and Serial Monitor
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    char buf[40];
    sprintf(buf, "Touch at rawX: %d rawY: %d z: %d", p.x, p.y, p.z);
    Serial.println(buf);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    sprintf(buf, "Touch at rawX: %d", p.x);
    tft.drawCentreString(buf, tft.width() / 2, tft.height() / 2 - 20, 2); // the touch point
    sprintf(buf, "Touch at rawY: %d", p.y);
    tft.drawCentreString(buf, tft.width() / 2, tft.height() / 2, 2); // the touch point
    sprintf(buf, "Touch at    z: %d", p.z);
    tft.drawCentreString(buf, tft.width() / 2, tft.height() / 2 + 20, 2); // the touch point
    delay(1000);
    imageIndex++;
    if (imageIndex > (MAX_IMAGES - 1)) imageIndex = 0;
    displayImage(fileNames[imageIndex]);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("Touch Image -> next", tft.width() / 2, 20, 2);
    tft.drawCentreString("Touch Image -> next", tft.width() / 2, 290, 2);
  }
}
