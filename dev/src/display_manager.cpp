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
draw_orientation_labels();
draw_axes();

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

// int deg = millis() / 10 % 360;

// float angle_rad = deg * DEG_TO_RAD;

// int effective_radius = radius - LABEL_MARGIN;

// int x = CENTER_X + (effective_radius-3) * cos(angle_rad);
// int y = CENTER_Y + (effective_radius-3) * sin(angle_rad);

// if (prev_x != 0 || prev_y != 0) {
//   tft->drawLine(CENTER_X, CENTER_Y, prev_x, prev_y, GC9A01A_BLACK);
// }
// tft->drawLine(CENTER_X, CENTER_Y, x, y, color);

draw_orientation_labels();

return 0;

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

void DisplayManager::draw_label
          (
          int         x,
          int         y,
          const char* text,
          uint16_t    color
          )
{
  tft->setTextSize(1);
  tft->setTextColor(color, GC9A01A_BLACK);
  tft->setCursor(x, y);
  tft->print(text);
}

void DisplayManager::persist_scope()
{

int effective_radius = RADIUS - LABEL_MARGIN;
for (int i = (effective_radius / RADAR_LINES); i <= effective_radius; i += (effective_radius / RADAR_LINES)) {
  tft->drawCircle(CENTER_X, CENTER_Y, i - 1, GC9A01A_GREEN);
}

}

void DisplayManager::draw_flight
          (
          int         x,
          int         y,
          const char* callsign,
          uint16_t    color
          )
{
  if (callsign == nullptr || callsign[0] == '\0') {
    return;
  }

  persist_scope();
  draw_axes();

  uint16_t marker_color = (color == GC9A01A_BLACK) ? GC9A01A_BLACK : GC9A01A_RED;
  tft->fillRect(x - 2, y - 2, 5, 5, marker_color);

  int label_x = x + 3;
  int label_y = y - 3;
  bool in_compass_zone = (label_x < 18) || (label_x > MAX_X - 24) || (label_y < 16) || (label_y > MAX_Y - 16);

  if (!in_compass_zone) {
    draw_label(label_x, label_y, callsign, color);
  }
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

  tft->setTextSize(1);
  tft->setTextColor(GC9A01A_WHITE);

  tft->setCursor(CENTER_X-6, 2);
  tft->print(top);

  tft->setCursor(CENTER_X-6, MAX_Y-10);
  tft->print(bottom);

  tft->setCursor(2, CENTER_Y-8);
  tft->print(left);

  tft->setCursor(MAX_X-8, CENTER_Y-8);
  tft->print(right);
}

void DisplayManager::draw_axes()
{
  tft->drawLine(0, CENTER_Y, MAX_X - 1, CENTER_Y, GC9A01A_GREEN);
  tft->drawLine(CENTER_X, 0, CENTER_X, MAX_Y - 1, GC9A01A_GREEN);

  int effective_radius = RADIUS - LABEL_MARGIN;
  for (int miles = 5; miles <= 20; miles += 5) {
    int pixel_radius = (int)((miles / 20.0f) * effective_radius);
    int x = CENTER_X - pixel_radius;
    int y = CENTER_Y;

    char label[8];
    sprintf(label, "%d", miles);

    tft->setTextSize(1);
    tft->setTextColor(GC9A01A_WHITE, GC9A01A_BLACK);
    tft->setCursor(x - 10, y + 4);
    tft->print(label);
  }
}

void DisplayManager::setOrientation(int deg)
{
  orientation = deg;
  draw_orientation_labels();
}

int DisplayManager::getOrientation()
{
  return orientation;
}
