#include "flight_manger.h"
#include "utils.h"
#include <string.h>

float center_lat = 38.9138863;
float center_lon = -94.7957893;
float RADIUS_MILES = 50.0;
float RADIUS_KM = RADIUS_MILES * 1.60934;

FlightManager::FlightManager() 
{
http_client = new HTTPClient();
}

FlightManager::~FlightManager() 
{
http_client->end();
delete http_client;
}

void FlightManager::update_flights(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km) 
{
    // Clear the current list of flights
    flights.clear();

    // Fetch new flight data from the API
    String payload = fetch_flight_data(wifi_client, center_lat, center_lon, radius_km);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err)
    {
        Serial.println("JSON parse failed");
        return;
    }
    JsonArray states = doc["states"];
    for (JsonArray plane : states)
    {
        const char* callsign = plane[1];
        float planeLat = plane[6];
        float planeLon = plane[5];

        // Check if the plane is within the specified radius
        float distance = sqrt(pow(planeLat - center_lat, 2) + pow(planeLon - center_lon, 2)) * 111; // Approximate conversion to km
        if (distance <= radius_km)
        {
            Flight flight;
            strncpy(flight.callsign, callsign ? callsign : "UNKNOWN", sizeof(flight.callsign) - 1);
            flight.latitude = planeLat;
            flight.longitude = planeLon;
            flights.push_back(flight);
        }
    }

}

void FlightManager::update_simulated_flights()
{
    flights.clear();

    const char* simulated_callsigns[] = {"SIM01", "SIM02", "SIM03", "SIM04"};
    const int flight_count = 4;
    unsigned long now = millis();
    float phase = (now / 1000.0f) * 0.8f;

    for (int i = 0; i < flight_count; ++i)
    {
        Flight flight;
        float angle = phase + (i * 0.9f);
        float lat_offset = sin(angle) * 0.18f;
        float lon_offset = cos(angle * 1.15f) * 0.22f;

        strncpy(flight.callsign, simulated_callsigns[i], sizeof(flight.callsign) - 1);
        flight.callsign[sizeof(flight.callsign) - 1] = '\0';
        flight.latitude = center_lat + lat_offset;
        flight.longitude = center_lon + lon_offset;
        flights.push_back(flight);
    }
}

String FlightManager::fetch_flight_data(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km) 
{

DEBUG("fetching ADS-B api");
float min_lat, max_lat, min_lon, max_lon;
calculate_bounding_box(center_lat, center_lon, radius_km, min_lat, max_lat, min_lon, max_lon);
//The OpenSky Network, https://opensky-network.org
http_client->begin(wifi_client, "https://opensky-network.org/api/states/all?lamin=" + String(min_lat) + 
                                                                          "&lomin=" + String(min_lon) + 
                                                                          "&lamax=" + String(max_lat) + 
                                                                          "&lomax=" + String(max_lon));
int code = http_client->GET();
if(code!=200)
{
  DEBUG("Failed to fetch data from ADS-B API");
  DEBUG("HTTP code: " + String(code));
  while(true);
}

return http_client->getString();

}

std::list<Flight> FlightManager::get_flights()
{

return flights;

}

static bool flights_equal(const Flight& a, const Flight& b)
{
    return strcmp(a.callsign, b.callsign) == 0
        && a.latitude == b.latitude
        && a.longitude == b.longitude;
}

void FlightMapper::map_flights()
{
    std::list<Flight> flights = flight_manager->get_flights();
    bool same_flights = flights.size() == previous_flights.size();

    if (same_flights) {
        auto it_new = flights.begin();
        auto it_old = previous_flights.begin();
        while (it_new != flights.end() && it_old != previous_flights.end()) {
            if (!flights_equal(*it_new, *it_old)) {
                same_flights = false;
                break;
            }
            ++it_new;
            ++it_old;
        }
    }

    if (!same_flights) {
        for (const Flight& old_flight : previous_flights) {
            bool erase_old = true;
            for (const Flight& new_flight : flights) {
                if (strcmp(old_flight.callsign, new_flight.callsign) == 0) {
                    if (flights_equal(old_flight, new_flight)) {
                        erase_old = false;
                    }
                    break;
                }
            }

            if (erase_old) {
                float x = (old_flight.longitude - center_lon) * 111;
                float y = (old_flight.latitude - center_lat) * 111;
                int screen_x = (int)(x / RADIUS_KM * 120 + 120);
                int screen_y = (int)(120 - (y / RADIUS_KM * 120));
                display_manager->draw_flight(screen_x, screen_y, old_flight.callsign, GC9A01A_BLACK);
            }
        }
    }

    for (const Flight& flight : flights)
    {
        float x = (flight.longitude - center_lon) * 111; // Approximate conversion to km
        float y = (flight.latitude - center_lat) * 111; // Approximate conversion to km
        int screen_x = (int)(x / RADIUS_KM * 120 + 120); // Map to screen coordinates
        int screen_y = (int)(120 - (y / RADIUS_KM * 120)); // Invert Y so higher latitude maps toward the top of the display
        display_manager->draw_flight(screen_x, screen_y, flight.callsign, GC9A01A_WHITE);
    }

    previous_flights = flights;
}