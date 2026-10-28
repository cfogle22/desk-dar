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
                        int         tft_rst,
                        int         tft_sck,
                        int         tft_miso,
                        int         tft_mosi
                        )
{

TFT_CS = tft_cs;
TFT_DC = tft_dc;
TFT_RST = tft_rst;
TFT_SCK = tft_sck;
TFT_MISO = tft_miso;
TFT_MOSI = tft_mosi;
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

SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
tft->begin();
tft->setRotation(0);
tft->fillScreen(GC9A01A_BLACK);
startup_active = true;
startup_animation();

}

void DisplayManager::finish_startup()
{
  startup_active = false;
  startup_logo_drawn = false;
  tft->fillScreen(GC9A01A_BLACK);
  draw_orientation_labels();
  persist_scope();
  draw_axes();
}

void DisplayManager::startup_animation(
                                      )
{
  if (startup_logo_drawn) {
    return;
  }

  const int fade_steps = 24;
  const int center_y = 94;
  const uint16_t cyan = tft->color565(0, 220, 255);
  const uint16_t green = tft->color565(0, 220, 110);
  const uint16_t dim_green = tft->color565(0, 85, 48);
  const uint16_t white = tft->color565(230, 245, 255);

  auto fade_color = [](uint16_t color, int step, int steps) -> uint16_t {
    uint16_t red = ((color >> 11) & 0x1F) * step / steps;
    uint16_t green = ((color >> 5) & 0x3F) * step / steps;
    uint16_t blue = (color & 0x1F) * step / steps;
    return (red << 11) | (green << 5) | blue;
  };

  for (int step = 1; step <= fade_steps; ++step) {
    tft->fillScreen(GC9A01A_BLACK);

    tft->drawCircle(CENTER_X, center_y, 54, fade_color(dim_green, step, fade_steps));
    tft->drawCircle(CENTER_X, center_y, 39, fade_color(dim_green, step, fade_steps));
    tft->drawCircle(CENTER_X, center_y, 23, fade_color(green, step, fade_steps));
    tft->drawLine(CENTER_X - 54, center_y, CENTER_X + 54, center_y, fade_color(dim_green, step, fade_steps));
    tft->drawLine(CENTER_X, center_y - 54, CENTER_X, center_y + 54, fade_color(dim_green, step, fade_steps));
    tft->drawLine(CENTER_X, center_y, CENTER_X + 37, center_y - 37, fade_color(cyan, step, fade_steps));
    tft->fillCircle(CENTER_X + 17, center_y - 18, 3, fade_color(cyan, step, fade_steps));
    tft->fillCircle(CENTER_X - 28, center_y + 12, 2, fade_color(green, step, fade_steps));
    tft->fillCircle(CENTER_X + 5, center_y + 38, 2, fade_color(green, step, fade_steps));

    tft->setTextSize(2);
    tft->setTextColor(fade_color(white, step, fade_steps));
    tft->setCursor(CENTER_X - 42, 164);
    tft->print("DESKDAR");
    tft->setTextSize(1);
    tft->setTextColor(fade_color(cyan, step, fade_steps));
    tft->setCursor(CENTER_X - 42, 187);
    tft->print("AIRSPACE RADAR");

    delay(25);
  }

  startup_logo_drawn = true;
}

void DisplayManager::initializing_display
                        (

                        )
{

  if (startup_active) {
    startup_animation();
  }
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

if (startup_active) return;
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
          float       heading,
          float       altitude,
          uint16_t    color
          )
{
  if (callsign == nullptr || callsign[0] == '\0') {
    return;
  }

  uint16_t marker_color = (color == GC9A01A_BLACK) ? GC9A01A_BLACK : GC9A01A_RED;
  bool heading_valid = isfinite(heading) && heading >= 0.0f && heading <= 360.0f;

  // Unknown headings use a circle marker and a neutral label direction.
  int size = 5; // smaller triangle
  float rad = 0.0f;
  float nose_x = x + size;
  float nose_y = y;
  float b1x = x - size;
  float b1y = y - size;
  float b2x = x - size;
  float b2y = y + size;
  int line_len = 6; // shorter nose line

  if (heading_valid) {
    // OpenSky headings are clockwise from north; convert to screen coordinates.
    float display_heading = heading - orientation;
    rad = (90.0f - display_heading) * DEG_TO_RAD;

    nose_x = x + cos(rad) * size;
    nose_y = y - sin(rad) * size;

    float base_angle = 120.0f * DEG_TO_RAD;
    float base_dist = size * 0.8f;
    b1x = x + cos(rad + base_angle) * base_dist;
    b1y = y - sin(rad + base_angle) * base_dist;
    b2x = x + cos(rad - base_angle) * base_dist;
    b2y = y - sin(rad - base_angle) * base_dist;

    tft->fillTriangle((int)nose_x, (int)nose_y, (int)b1x, (int)b1y, (int)b2x, (int)b2y, marker_color);
    tft->drawTriangle((int)nose_x, (int)nose_y, (int)b1x, (int)b1y, (int)b2x, (int)b2y, marker_color);

    float nose2x = x + cos(rad) * (size + line_len);
    float nose2y = y - sin(rad) * (size + line_len);
    tft->drawLine((int)nose_x, (int)nose_y, (int)nose2x, (int)nose2y, marker_color);
  } else {
    tft->fillCircle(x, y, size - 1, marker_color);
    tft->drawCircle(x, y, size, marker_color);
  }

    // Prepare label lines
    char line1[16];
    char line2[24];
    snprintf(line1, sizeof(line1), "%s", callsign);
    size_t callsign_length = strlen(line1);
    while (callsign_length > 0 && line1[callsign_length - 1] == ' ') {
      line1[--callsign_length] = '\0';
    }
    if (altitude > 0.0f) {
      snprintf(line2, sizeof(line2), "%.0ft", altitude * 3.28084f); // Convert meters to feet
    } else {
      snprintf(line2, sizeof(line2), "NO_ALT");
    }

    // The default font advances 6 pixels per character at textSize 1.
    int max_len = max((int)strlen(line1), (int)strlen(line2));
    int label_w = max_len * 6;
    int label_h = 8 * 2; // 2 lines of 8px
    const int label_padding = 2;

      // Position label along the nose direction, offset to the right of heading
      int along_dist = size + line_len + 2; // bring labels closer
      float along_x = cos(rad) * along_dist;
      float along_y = -sin(rad) * along_dist;
      float perp_x = cos(rad + (M_PI / 2.0f)) * 4.0f; // smaller side offset
      float perp_y = -sin(rad + (M_PI / 2.0f)) * 4.0f;

    int label_x = (int)(x + along_x + perp_x);
    int label_y = (int)(y + along_y + perp_y - (label_h / 2));

    // If label would be off-screen, flip to the other side of the nose
    if (label_x - label_padding < 0 || label_x + label_w + label_padding > MAX_X) {
      // flip to left side
      label_x = (int)(x + along_x - perp_x - label_w);
    }
    label_x = constrain(label_x, label_padding, MAX_X - label_w - label_padding);
    label_y = constrain(label_y, label_padding, MAX_Y - label_h - label_padding);

    // If label intersects triangle bounding box, push it further out
    int tri_min_x = (int)min(nose_x, min(b1x, b2x));
    int tri_max_x = (int)max(nose_x, max(b1x, b2x));
    int tri_min_y = (int)min(nose_y, min(b1y, b2y));
    int tri_max_y = (int)max(nose_y, max(b1y, b2y));

    auto rects_intersect = [&](int rx, int ry, int rw, int rh){
      return !(rx + rw < tri_min_x || rx > tri_max_x || ry + rh < tri_min_y || ry > tri_max_y);
    };

    if (rects_intersect(label_x - label_padding, label_y - label_padding, label_w + label_padding * 2, label_h + label_padding * 2)) {
      // move label slightly further along the nose direction to avoid overlapping
      int push = 12; // fixed small push
      label_x += (int)(cos(rad) * push);
      label_y += (int)(-sin(rad) * push);
      // if still off-screen, flip side
      if (label_x - label_padding < 0 || label_x + label_w + label_padding > MAX_X) {
        label_x = (int)(x + along_x - perp_x - label_w);
      }
      label_x = constrain(label_x, label_padding, MAX_X - label_w - label_padding);
      label_y = constrain(label_y, label_padding, MAX_Y - label_h - label_padding);
    }

    // Background for label
    tft->fillRect(label_x - label_padding, label_y - label_padding, label_w + label_padding * 2, label_h + label_padding * 2, GC9A01A_BLACK);

    tft->setTextSize(1);
    // print callsign in provided color
    tft->setTextColor(color, GC9A01A_BLACK);
    tft->setCursor(label_x, label_y);
    tft->print(line1);
    // print altitude in cyan
    tft->setTextColor(GC9A01A_CYAN, GC9A01A_BLACK);
    tft->setCursor(label_x, label_y + 8);
    tft->print(line2);
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
