#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>
#include <list>
#include "display_manager.h"
#include "flight_manger.h"
#include "utils.h"
#include "configuration.h"
#include "display_pins.h"

#define SIMULATE_FLIGHTS 0

#if !SIMULATE_FLIGHTS
#include "network_manager.h"
#endif

#define DEBUG(X) Serial.println(X)

DisplayManager display_manager(
    TFT_CS_PIN,
    TFT_DC_PIN,
    TFT_RST_PIN,
    TFT_SCK_PIN,
    TFT_MISO_PIN,
    TFT_MOSI_PIN
);
FlightManager flight_manager;
FlightMapper flight_mapper(&display_manager, &flight_manager);
const unsigned long FLIGHT_UPDATE_INTERVAL_MS = 15000UL;
unsigned long last_flight_update_ms = 0;
bool startup_complete = false;

#if !SIMULATE_FLIGHTS
NetworkManager network_manager;
bool network_connected = false;
#endif

void setup() {
  Serial.begin(115200);
  display_manager.begin();

  DEBUG("Starting up");
  DEBUG("Initializing display");

  last_flight_update_ms = millis() - FLIGHT_UPDATE_INTERVAL_MS;

#if SIMULATE_FLIGHTS
  flight_manager.update_simulated_flights();
  display_manager.finish_startup();
  startup_complete = true;
  flight_mapper.map_flights();
#else
  network_connected = network_manager.connect();
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
    network_connected = network_manager.connect();
  }

  // flight_manager.update_flights(network_manager.get_client(), center_lat, center_lon, RADIUS_KM);
#endif

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
  const unsigned long now = millis();
  if (now - last_flight_update_ms >= FLIGHT_UPDATE_INTERVAL_MS) {
    last_flight_update_ms = now;
    if (!network_connected) {
      network_connected = network_manager.connect();
      if (!network_connected) {
        return;
      }
    }

    DEBUG("Updating flight data");
    bool received_batch = flight_manager.update_flights(
        network_manager.get_client(),
        center_lat,
        center_lon,
        RADIUS_KM
    );
    DEBUG("Flight data updated");
    if (received_batch) {
      if (!startup_complete) {
        display_manager.finish_startup();
        startup_complete = true;
      }
      flight_mapper.map_flights();
    }
  }
#endif
}