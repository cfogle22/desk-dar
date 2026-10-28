#ifndef DISPLAY_PINS_H
#define DISPLAY_PINS_H

#if defined(DESKDAR_C3_SUPERMINI)
// Wire the TFT to GPIO 4 (SCK), 6 (MOSI), 7 (CS), 10 (DC), and 3 (RST).
static constexpr int TFT_CS_PIN = 7;
static constexpr int TFT_DC_PIN = 10;
static constexpr int TFT_RST_PIN = 3;
static constexpr int TFT_SCK_PIN = 4;
static constexpr int TFT_MISO_PIN = -1;
static constexpr int TFT_MOSI_PIN = 6;
#else
static constexpr int TFT_CS_PIN = 5;
static constexpr int TFT_DC_PIN = 27;
static constexpr int TFT_RST_PIN = 33;
static constexpr int TFT_SCK_PIN = 18;
static constexpr int TFT_MISO_PIN = 19;
static constexpr int TFT_MOSI_PIN = 23;
#endif

#endif
