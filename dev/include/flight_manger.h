#ifndef FLIGHT_MANAGER
#define FLIGHT_MANAGER

#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <math.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "display_manager.h"
#include <list>

float center_lat = 39.0;
float center_lon = -94.7;

float radius_miles = 100.0;
float radius_km = radius_miles * 1.60934;

typedef struct {
    char callsign[9];
    float latitude;
    float longitude;
} Flight;

class FlightManager 
{

HTTPClient* http_client;
std::list<Flight> flights;

    public:

FlightManager();
~FlightManager();

void update_flights(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km);

String fetch_flight_data(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km);

    private:
void calculate_bounding_box(float center_lat, float center_lon, float radius_km, float& min_lat, float& max_lat, float& min_lon, float& max_lon) 
{

float lat_rad = center_lat * DEG_TO_RAD;
float lon_rad = center_lon * DEG_TO_RAD;
float angular_distance = radius_km / 6371.0; // Earth's radius in km

min_lat = center_lat - (angular_distance * RAD_TO_DEG);
max_lat = center_lat + (angular_distance * RAD_TO_DEG);
min_lon = center_lon - (angular_distance * RAD_TO_DEG / cos(lat_rad));
max_lon = center_lon + (angular_distance * RAD_TO_DEG / cos(lat_rad));

};

};

class FlightMapper
{

public:



};

#endif