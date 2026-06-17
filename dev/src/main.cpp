#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33
#define CENTER_Y 120
#define CENTER_X 120
#define RADAR_LINES 4
#define MAX_X 240
#define MAX_Y 240
#define RADIUS 120

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);
#define DEBUG(X) Serial.println(X)

void do_radar_screen(Adafruit_GC9A01A *tft) {
  tft->setCursor(CENTER_X,CENTER_Y);
  for(int i = (RADIUS/RADAR_LINES); i<=RADIUS; i+=(RADIUS/RADAR_LINES)) {
    tft->drawCircle(CENTER_X,CENTER_Y,i-1, GC9A01A_GREEN);
  }
}

uint16_t deg = 0;

void sweeping_line_update(Adafruit_GC9A01A *tft) {
  if(deg>360)
  tft->drawLine(CENTER_X,CENTER_Y, 120 * cos(deg), 120 * sin(deg), GC9A01A_GREEN);
  deg++;
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  DEBUG("Starting up");
  DEBUG("Initilizing display");
  tft.begin();
  // Rotate if needed (0-3)
  tft.setRotation(0);

  // Fill screen black
  tft.fillScreen(GC9A01A_BLACK);

    // Draw some stuff
  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.setCursor(CENTER_X, CENTER_Y);
  tft.println("Hello!");
  delay(1000);
  tft.fillScreen(GC9A01A_BLACK);
  delay(1000);
  tft.setCursor(CENTER_X, CENTER_Y);
  tft.println("Hello!");
  delay(1000);
  tft.fillScreen(GC9A01A_BLACK);
  delay(1000);
  tft.setCursor(CENTER_X, CENTER_Y);
  tft.println("Hello!");
  delay(1000);
  tft.fillScreen(GC9A01A_BLACK);
  delay(1000);

  do_radar_screen(&tft);
}

void loop() {
  sweeping_line_update(&tft);
}