#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>
#include "display_manager.h"
#include "network_manager.h"
#include "utils.h"

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33


#define DEBUG(X) Serial.println(X)
DisplayManager display_manager(TFT_CS, TFT_DC, TFT_RST);
NetworkManager network_manager;

void setup() {

unsigned long start_time = millis();
Serial.begin(115200);
while(!Serial);
DEBUG("Starting up");
DEBUG("Initializing display");
display_manager.begin();
display_manager.cls();
unsigned long now = millis();
display_manager.initializing_display();
if(!network_manager.connect())
{
  DEBUG("Failed to connect to WiFi, no saved network config?");
  DEBUG("Please set up the network config and try again");
  DEBUG("ENTER NETWORK NAME:");
  network_config_t config;
  String input = serial_read_string();
  strncpy(config.ssid, input.c_str(), sizeof(config.ssid));
  config.ssid[sizeof(config.ssid) - 1] = '\0';
  DEBUG("ENTER NETWORK PASSWORD:");
  input = serial_read_string();
  strncpy(config.password, input.c_str(), sizeof(config.password));
  config.password[sizeof(config.password) - 1] = '\0';

  network_manager.set(config);
}

display_manager.cls();
start_time = millis() - start_time;
while(millis() - start_time < 3000) {
display_manager.initializing_display();
delay(100);
}
display_manager.cls();

DEBUG("Attempting another WiFi connect");
if(!network_manager.connect())
{
  DEBUG("Failed to connect to WiFi with provided config");
  DEBUG("Please check the credentials and try again");
  display_manager.print("Failed to connect to WiFi\n");
  display_manager.print("Please check the credentials and try again");
  while(true);
}

display_manager.cls();
start_time = millis() - start_time;
while(millis() - start_time < 3000) {
display_manager.initializing_display();
delay(100);
}

display_manager.cls();



DEBUG("fetching ADS-B api");
http_client.begin(network_manager.get_client(), "https://opensky-network.org/api/states/all?lamin=38.8&lomin=-95.0&lamax=39.2&lomax=-94.4");
int code = http_client.GET();
if(code!=200)
{
  DEBUG("Failed to fetch data from ADS-B API");
  Serial.printf("HTTP error code: %d\n", code);
  display_manager.print("Failed to fetch data from ADS-B API");
  while(true);
}

DEBUG("Data fetched successfully from ADS-B API");
String payload = http_client.getString();
StaticJsonDocument<16000> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (err)
    {
        Serial.println("JSON parse failed");
        return;
    }

    JsonArray states = doc["states"];

    Serial.printf("Planes found: %d\n", states.size());

    for (JsonArray plane : states)
    {
        const char* callsign = plane[1];
        float planeLat = plane[6];
        float planeLon = plane[5];

        Serial.print("Callsign: ");
        Serial.print(callsign ? callsign : "UNKNOWN");
        Serial.print(" | Lat: ");
        Serial.print(planeLat, 4);
        Serial.print(" | Lon: ");
        Serial.println(planeLon, 4);
    }

    http_client.end();
}

void loop() {
  DEBUG(".");
  display_manager.do_radar_screen(RADIUS, GC9A01A_GREEN);
}