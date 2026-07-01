#include "display_manager.h"

// define static instance
DisplayManager* DisplayManager::instance = nullptr;

DisplayManager* DisplayManager::getInstance(

                                           )
{

return DisplayManager::instance;

}

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

DisplayManager::instance = this;

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
// clear singleton instance before destroying
DisplayManager::instance = nullptr;
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

float DisplayManager::do_radar_screen
                    (
                    int         radius,
                    uint16_t    color
                    )
{

int deg = millis() / 10 % 360;

float angle_rad = deg * DEG_TO_RAD;

int effective_radius = radius - LABEL_MARGIN;

int x = CENTER_X + effective_radius * cos(angle_rad);
int y = CENTER_Y + effective_radius * sin(angle_rad);

tft->drawLine(CENTER_X, CENTER_Y, x, y, color);
tft->drawLine(CENTER_X, CENTER_Y, prev_x, prev_y, GC9A01A_BLACK);

tft->setCursor(CENTER_X,CENTER_Y);
for( int i = ( effective_radius/RADAR_LINES ) ; i <= effective_radius; i += ( effective_radius/RADAR_LINES ) ) 
{
  //Draw radar circles everytime
  tft->drawCircle(CENTER_X,CENTER_Y,i-1, color);
}

draw_orientation_labels();

prev_x = x;
prev_y = y;

return deg;

}

void DisplayManager::print
                    (
                        const char* text
                    )
{

tft->setTextColor(GC9A01A_WHITE);
tft->print(text);

}

void DisplayManager::draw_pixel
          (
          int         x,
          int         y,
          uint16_t    color
          )
{
tft->drawPixel(x, y, color);
}

void DisplayManager::draw_orientation_labels()
{
  auto bearingToLabel = [](int deg)->char {
    deg = ((deg % 360) + 360) % 360;
    if (deg >= 315 || deg < 45) return 'N';
    if (deg >= 45 && deg < 135) return 'E';
    if (deg >= 135 && deg < 225) return 'S';
    return 'W';
  };

  char top = bearingToLabel(orientation);
  char right = bearingToLabel(orientation + 90);
  char bottom = bearingToLabel(orientation + 180);
  char left = bearingToLabel(orientation + 270);

  // clear small areas where labels will be drawn
  tft->fillRect(CENTER_X-8, 0, 16, LABEL_MARGIN+1, GC9A01A_BLACK); // top
  tft->fillRect(CENTER_X-8, MAX_Y-(LABEL_MARGIN+1), 16, LABEL_MARGIN+1, GC9A01A_BLACK); // bottom
  tft->fillRect(0, CENTER_Y-8, LABEL_MARGIN+1, 16, GC9A01A_BLACK); // left
  tft->fillRect(MAX_X-(LABEL_MARGIN+1), CENTER_Y-8, LABEL_MARGIN+1, 16, GC9A01A_BLACK); // right

  tft->setTextSize(1);
  tft->setTextColor(GC9A01A_WHITE);

  tft->setCursor(CENTER_X-6, 2);
  tft->print(top);

  tft->setCursor(CENTER_X-6, MAX_Y-16);
  tft->print(bottom);

  tft->setCursor(2, CENTER_Y-6);
  tft->print(left);

  tft->setCursor(MAX_X-14, CENTER_Y-6);
  tft->print(right);
}
