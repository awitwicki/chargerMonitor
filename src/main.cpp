#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <GyverINA.h>
#include "mString.h"
#include <SPI.h>

#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8
#define TFT_MOSI 11  // Data out
#define TFT_SCLK 13  // Clock out

//INA226 ina;
mString<34> str;
int32_t _seconds = 0;
int energy = 9000;

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void drawLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  tft.drawFastVLine(x, y + 32, w, color);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  tft.drawRect(x, y + 32, w, h, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  tft.fillRect(x, y + 32, w, h, color);
}

void drawData(int x, float current, float power, float voltage, int energy, int32_t seconds) {
  int y = 32;
  int step = 15;

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);

  // Current
  y += 10;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + current + " A";
  tft.print(str.c_str());

  // Voltage
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + voltage + " V";
  tft.print(str.c_str());

  // Charge power
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + power + " W";
  tft.print(str.c_str());

  // Stored energy
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + energy + " mAh";
  tft.print(str.buf);

  // Time
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + seconds/60 + ":" + seconds % 60;
  tft.print(str.buf);
}

void updateInterface() {
  // tft.fillScreen(ST77XX_BLACK);

  // Draw borders
  drawRect(0,0,128,128, ST77XX_GREEN);
  drawRect(1,1,127,127, ST77XX_GREEN);
  drawLine(65, 0, 128, ST77XX_GREEN);
  drawLine(66, 0, 128, ST77XX_GREEN);

  // float current = ina.getCurrent();
  // float power = ina.getPower();
  // float voltage = ina.getShuntVoltage();

  float current = 0.895;
  float power = 4.363;
  float voltage = 3.668;

  drawData(10, current, power, voltage, energy, _seconds);
  drawData(75, current, power, voltage, energy, _seconds);
}

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // Clear display
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  updateInterface();

  delay(1000);

  _seconds++;
  energy++;
}
