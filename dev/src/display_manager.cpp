#include "display_manager.h"

DisplayManager::DisplayManager
                        (
                        int         tft_cs,
                        int         tft_dc,
                        int         tft_rst
                        )
{

 TFT_CS = tft_cs;
 TFT_DC = tft_dc;
 TFT_RST = tft_rst;
 tft = new Adafruit_GC9A01A(TFT_CS, TFT_DC, TFT_RST);

}

DisplayManager::~DisplayManager
                        (
                        )
{
tft->fillScreen(GC9A01A_BLACK);
tft->setCursor(CENTER_X-30, CENTER_Y-30);
tft->setTextColor(GC9A01A_WHITE);
tft->setTextSize(2);
tft->print("Shutting down");
delay(1000);
delete tft;
}

void DisplayManager::begin
                        (
                        )
{

SPI.begin();
tft->begin();
tft->setRotation(0);

}

void DisplayManager::initializing_display
                        (

                        ) 
{

tft->setTextColor(GC9A01A_WHITE);
tft->setTextSize(2);
tft->setCursor(CENTER_X-100, CENTER_Y);
tft->print("Initializing  ");

// Clear the spinner character area
tft->fillRect(CENTER_X+50, CENTER_Y, 12, 20, GC9A01A_BLACK);

// Draw the spinner character
tft->setCursor(CENTER_X+50, CENTER_Y);
tft->setTextColor(GC9A01A_WHITE);
tft->print(loading_states[loading_state_index]);

loading_state_index = (loading_state_index + 1) % 4;
}

void DisplayManager::do_radar_screen
                    (
                    int         radius,
                    uint16_t    color
                    )
{

int deg = millis() / 10 % 360;

float angle_rad = deg * DEG_TO_RAD;

int x = CENTER_X + radius * cos(angle_rad);
int y = CENTER_Y + radius * sin(angle_rad);

tft->drawLine(CENTER_X, CENTER_Y, x, y, color);
tft->drawLine(CENTER_X, CENTER_Y, prev_x, prev_y, GC9A01A_BLACK);

tft->setCursor(CENTER_X,CENTER_Y);
for( int i = ( radius/RADAR_LINES ) ; i <= radius; i += ( radius/RADAR_LINES ) ) 
{
  //Draw radar circles everytime
  tft->drawCircle(CENTER_X,CENTER_Y,i-1, color);
}

prev_angle = deg;
prev_radius = radius;
prev_x = x;
prev_y = y;

}

