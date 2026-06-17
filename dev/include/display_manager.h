#ifndef DISPLAY_MANAGER

#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33
#define CENTER_Y 120
#define CENTER_X 120

class DisplayManager {

    //default to adafruit limits
    uint16_t MAX_X = 240;
    uint16_t MAX_Y = 240;

    DisplayManager(uint16_t max_x, uint16_t max_y) {

    }
    ~DisplayManager(){}

};

#endif