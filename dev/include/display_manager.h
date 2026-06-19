#ifndef DISPLAY_MANAGER
#define DISPLAY_MANAGER

#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define CENTER_Y 120
#define CENTER_X 120
#define RADAR_LINES 4
#define MAX_X 240
#define MAX_Y 240
#define RADIUS 120
class DisplayManager {

Adafruit_GC9A01A*   tft;

int TFT_CS;
int TFT_DC;  
int TFT_RST;

    public:

DisplayManager  
                    ( 
                    int         tft_cs,
                    int         tft_dc,
                    int         tft_rst
                    );

~DisplayManager();

void begin();

void initializing_display
                    (

                    );

void print
                    (
                        const char* text
                    );

float do_radar_screen
                    (
                    int         radius,
                    uint16_t    color
                    );

void cls            (

                    )
{

tft->fillScreen(GC9A01A_BLACK);

}

static DisplayManager* getInstance();

static DisplayManager* instance;

    private:

int     prev_x                  = 0;
int     prev_y                  = 0;

char    loading_states[8]       = {'|'  , '/' , '-' , '\\'};
int     loading_state_index     = 0;

};

#endif