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

INA219 ina_1((uint8_t)0x40);
INA219 ina_2((uint8_t)0x41);

mString<34> str;
int32_t _seconds1 = 0;
int32_t _seconds2 = 0;

// Global char buffer for the printing of seconds in formatted time
char formattedSecondsBuffer[14];

struct SensorStoredData {
  float energy = 0;
  float current = 0;
  float power = 0;
  float voltage = 0;
};

// Sensors data
SensorStoredData sensor_1;
SensorStoredData sensor_2;

// Timer variable
unsigned long stopwatchStart;
unsigned long executionTime;

char* getFormattedSecondsString(uint32_t allSeconds) {
  uint16_t hours            = allSeconds / 3600;    // convert seconds to hours
  uint16_t secondsRemaining = allSeconds % 3600;    // seconds left over

  uint16_t minutes  = secondsRemaining / 60 ;       // convert seconds left over to minutes
  uint16_t seconds  = secondsRemaining % 60;        // seconds left over

  // "prints" formatted output to a char array (string)
  snprintf(formattedSecondsBuffer, sizeof(formattedSecondsBuffer),
            "%02d:"   //HH:
            "%02d:"   //MM:
            "%02d"   //SS
            ,
            hours,
            minutes,
            seconds
          );

  return formattedSecondsBuffer;
}

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

void drawData(int x, float _current, float _power, float _voltage, float _energy, int32_t seconds) {
  int y = 32;
  int step = 15;

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);

  // Current
  y += 6;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + _current + " A";
  tft.print(str.c_str());

  // Voltage
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + _voltage + " V";
  tft.print(str.c_str());

  // Charge power
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  str = str + _power + " W";
  tft.print(str.c_str());

  // Stored energy
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = "";
  if (_energy > 10) {
    str += int(_energy);
  } else {
    str += _energy;
  }
  str += + " mAh";
  tft.print(str.buf);

  // Time
  y += step;
  tft.fillRect(x, y, 48, 7, ST77XX_BLACK); // Clear screen zone
  tft.setCursor(x, y);
  str = getFormattedSecondsString(seconds);
  tft.print(str.buf);
}

void updateInterface() {
  // tft.fillScreen(ST77XX_BLACK);

  // Draw borders
  drawRect(0, 0, 128, 128, ST77XX_GREEN);
  drawRect(1, 1, 127, 127, ST77XX_GREEN);
  drawLine(65, 0, 128, ST77XX_GREEN);
  drawLine(66, 0, 128, ST77XX_GREEN);

  sensor_1.current = ina_1.getCurrent();
  sensor_1.power = ina_1.getPower();
  sensor_1.voltage = ina_1.getShuntVoltage();

  sensor_2.current = ina_2.getCurrent();
  sensor_2.power = ina_2.getPower();
  sensor_2.voltage = ina_2.getShuntVoltage();

  drawData(7, sensor_1.current, sensor_1.power, sensor_1.voltage, sensor_1.energy, _seconds1);
  drawData(71, sensor_2.current, sensor_2.power, sensor_2.voltage, sensor_2.energy, _seconds2);
}

void setup(void) {
  stopwatchStart = micros();
  Serial.begin(9600);

  // Init Ina boards
  // 1st
  ina_1.begin();
  ina_1.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);
  ina_1.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128); 

  // 2nd
  ina_2.begin();
  ina_2.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);
  ina_2.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128);

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // Clear display
  tft.fillScreen(ST77XX_BLACK);

  executionTime = micros() - stopwatchStart;

  // Serial.print(F("Init time (microseconds)\t"));
  // Serial.println(executionTime);
  // _seconds += executionTime / 1000000;
}

void loop() {
  stopwatchStart = micros();
  updateInterface();

  sensor_1.energy += sensor_1.current * 1000 / 3600;
  sensor_2.energy += sensor_2.current * 1000 / 3600;

  executionTime = micros() - stopwatchStart;

  Serial.print(F("executionTime (microseconds)\t"));
  Serial.println(executionTime);

  unsigned long delayTimeMicros = 1000000;
  delayTimeMicros -= executionTime;

  // Serial.print(F("total deylaytim (microseconds)\t"));
  // Serial.println(delayTimeMicros);

  // Serial.print(F("delayTime (milliseconds)\t"));
  // Serial.println(delayTimeMicros / 1000);

  // Serial.print(F("delayTime (microseconds)\t"));
  // Serial.println(delayTimeMicros % 1000);

  // Delay milliseconds
  delay(delayTimeMicros / 1000);

  // Delay microseconds
  delayMicroseconds(delayTimeMicros % 1000);

  if (sensor_1.current > 0.01) {
    _seconds1++;
  }
  if (sensor_2.current > 0.01) {
    _seconds2++;
  }
}
