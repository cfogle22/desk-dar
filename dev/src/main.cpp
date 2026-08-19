#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>
#include <list>
#include "display_manager.h"
#include "flight_manger.h"
#include "utils.h"

#define SIMULATE_FLIGHTS 0

#if !SIMULATE_FLIGHTS
#include "network_manager.h"
#endif

#define TFT_CS   5
#define TFT_DC   27
#define TFT_RST  33

#define DEBUG(X) Serial.println(X)

DisplayManager display_manager(TFT_CS, TFT_DC, TFT_RST);
FlightManager flight_manager;
FlightMapper flight_mapper(&display_manager, &flight_manager);
const unsigned long FLIGHT_UPDATE_INTERVAL_MS = 15000UL;
unsigned long last_flight_update_ms = 0;

#if !SIMULATE_FLIGHTS
NetworkManager network_manager;
volatile bool flight_update_requested = false;
hw_timer_t* flight_timer = nullptr;

void IRAM_ATTR onFlightTimer() {
  flight_update_requested = true;
}
#endif

void setup() {
  unsigned long start_time = millis();
  Serial.begin(115200);
  while(!Serial);

  DEBUG("Starting up");
  DEBUG("Initializing display");

  last_flight_update_ms = millis();
  display_manager.begin();
  display_manager.cls();
  display_manager.initializing_display();

#if SIMULATE_FLIGHTS
  display_manager.print("Simulation mode");
  delay(1000);
  display_manager.cls();
  flight_manager.update_simulated_flights();
  flight_mapper.map_flights();
#else
bool network_connected = network_manager.connect();
  if(!network_connected)
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
  if(!network_connected)
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

  flight_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(flight_timer, &onFlightTimer, true);
  timerAlarmWrite(flight_timer, 15000000, true);
  timerAlarmEnable(flight_timer);

  flight_manager.update_flights(network_manager.get_client(), center_lat, center_lon, RADIUS_KM);
#endif

  flight_mapper.map_flights();
}

void loop() {
  // display_manager.do_radar_screen(RADIUS, GC9A01A_GREEN);

#if SIMULATE_FLIGHTS
  const unsigned long now = millis();
  if (now - last_flight_update_ms >= FLIGHT_UPDATE_INTERVAL_MS) {
    last_flight_update_ms = now;
    flight_manager.update_simulated_flights();
  }
#else
  if (flight_update_requested) {
    flight_update_requested = false;
    DEBUG("Updating flight data");
    display_manager.persist_scope();
    flight_manager.update_flights(network_manager.get_client(), center_lat, center_lon, RADIUS_KM);
    DEBUG("Flight data updated");
    flight_mapper.map_flights();
  }
#endif
}