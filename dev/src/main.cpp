#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>
#include "display_manager.h"
#include "network_manager.h"
#include "flight_manger.h"
#include "utils.h"

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33


#define DEBUG(X) Serial.println(X)
DisplayManager display_manager(TFT_CS, TFT_DC, TFT_RST);
NetworkManager network_manager;
FlightManager flight_manager;
FlightMapper flight_mapper(&display_manager, &flight_manager);

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

}

void loop() {
display_manager.do_radar_screen(RADIUS, GC9A01A_GREEN);
flight_manager.update_flights(network_manager.get_client(), center_lat, center_lon, RADIUS_KM);
for( Flight flight : flight_manager.get_flights())
{
  DEBUG("---------------------------------------------------");
  String callsign = "Callsign: " + String(flight.callsign);
  String lat_str = "Latitude: " + String(flight.latitude,4);
  String lon_str = "Longitude: " + String(flight.longitude, 4);
  DEBUG(callsign);
  DEBUG(lat_str);
  DEBUG(lon_str);
  DEBUG("---------------------------------------------------");
  flight_mapper.map_flights();
}
delay(10000);
}