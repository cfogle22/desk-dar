#include "flight_manger.h"
#include "utils.h"

FlightManager::FlightManager() 
{
http_client = new HTTPClient();
}

FlightManager::~FlightManager() 
{
delete http_client;
}

void FlightManager::update_flights(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km) 
{
    // Clear the current list of flights
    flights.clear();

    // Fetch new flight data from the API
    String payload = fetch_flight_data(wifi_client, center_lat, center_lon, radius_km);
    StaticJsonDocument<16000> doc;
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

String FlightManager::fetch_flight_data(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km) 
{

DEBUG("fetching ADS-B api");
float min_lat, max_lat, min_lon, max_lon;
calculate_bounding_box(center_lat, center_lon, radius_km, min_lat, max_lat, min_lon, max_lon);
http_client->begin(wifi_client, "https://opensky-network.org/api/states/all?lamin=" + String(min_lat) + 
                                                                          "&lomin=" + String(min_lon) + 
                                                                          "&lamax=" + String(max_lat) + 
                                                                          "&lomax=" + String(max_lon));
int code = http_client->GET();
if(code!=200)
{
  DEBUG("Failed to fetch data from ADS-B API");
  while(true);
}

return http_client->getString();

}