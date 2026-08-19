#ifndef DISPLAY_MANAGER
#define DISPLAY_MANAGER

#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define CENTER_Y 120
#define CENTER_X 120
#define RADAR_LINES 3
#define MAX_X 240
#define MAX_Y 240
#define RADIUS 120
#define LABEL_MARGIN 10
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

void draw_orientation_labels();
void draw_axes();

void cls            (

                    )
{

tft->fillScreen(GC9A01A_BLACK);
draw_orientation_labels();

}

void draw_pixel
                    (
                    int         x,
                    int         y,
                    uint16_t    color
                    );

void draw_label
                    (
                    int         x,
                    int         y,
                    const char* text,
                    uint16_t    color
                    );

void draw_flight
                    (
                    int         x,
                    int         y,
                    const char* callsign,
                    uint16_t    color
                    );

void persist_scope
                    (

                    );

void setOrientation(int deg);
int getOrientation();

static DisplayManager* getInstance();

static DisplayManager* instance;

    private:

int     prev_x                  = 0;
int     prev_y                  = 0;

int     orientation             = 0; // degrees clockwise from North
unsigned long last_label_millis = 0;

char    loading_states[8]       = {'|'  , '/' , '-' , '\\'};
int     loading_state_index     = 0;

};

#endif