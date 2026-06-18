#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>
#include "display_manager.h"

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33

#define DEBUG(X) Serial.println(X)
DisplayManager display_manager(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  unsigned long start_time = millis();
  Serial.begin(115200);
  while(!Serial);
  DEBUG("Starting up");
  DEBUG("Initializing display");
  display_manager.begin();
  display_manager.cls();
  unsigned long now = millis();
  while(now - start_time < 10000) {
    display_manager.initializing_display();
    delay(100);
    now = millis();
  }
  
  display_manager.cls();

}

void loop() {
  DEBUG(".");
  display_manager.do_radar_screen(RADIUS, GC9A01A_GREEN);
}