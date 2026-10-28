#include "flight_manger.h"
#include "utils.h"
#include "configuration.h"
#include <string.h>
#include <Arduino.h>

float center_lat = 38.9138863;
float center_lon = -94.7957893;
float RADIUS_MILES = 20.0;
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

bool FlightManager::update_flights(WiFiClientSecure& wifi_client, float center_lat, float center_lon, float radius_km)
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
        return false;
    }

    JsonVariant states_value = doc["states"];
    if (!doc["time"].is<unsigned long>() ||
        (!states_value.is<JsonArray>() && !states_value.isNull()))
    {
        Serial.println("Invalid OpenSky response");
        return false;
    }

    if (states_value.is<JsonArray>())
    {
        JsonArray states = states_value.as<JsonArray>();
        for (JsonArray plane : states)
        {
            const char* callsign = plane[1];
            float planeLat = plane[6];
            float planeLon = plane[5];
            JsonVariant heading_value = plane[10];
            float trueHeading = heading_value.is<float>()
                ? heading_value.as<float>()
                : NAN;
            float baroAltitude = plane[7];
            float geoAltitude = plane[13];

            // Check if the plane is within the specified radius
            float distance = sqrt(pow(planeLat - center_lat, 2) + pow(planeLon - center_lon, 2)) * 111; // Approximate conversion to km
            if (distance <= radius_km)
            {
                Flight flight;
                strncpy(flight.callsign, callsign ? callsign : "UNKNOWN", sizeof(flight.callsign) - 1);
                flight.callsign[sizeof(flight.callsign) - 1] = '\0';
                flight.latitude = planeLat;
                flight.longitude = planeLon;
                flight.true_heading = trueHeading;
                flight.baro_altitude = baroAltitude;
                flight.geo_altitude = geoAltitude;
                flights.push_back(flight);
            }
        }
    }

    return true;
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
        flight.true_heading = fmodf((angle * 180.0f / M_PI) + 360.0f, 360.0f);
        flight.geo_altitude = 3000.0f + i * 500.0f;
        flights.push_back(flight);
    }
}

String FlightManager::fetch_flight_data(
    WiFiClientSecure& wifi_client,
    float center_lat,
    float center_lon,
    float radius_km)
{
    DEBUG("Fetching OpenSky flight data");

    if (!ensure_access_token(wifi_client)) {
        Serial.println("Unable to authenticate with OpenSky");
        return "";
    }

    float min_lat, max_lat, min_lon, max_lon;

    calculate_bounding_box(
        center_lat,
        center_lon,
        radius_km,
        min_lat, max_lat, min_lon, max_lon
    );

    String url =
        "https://opensky-network.org/api/states/all?lamin=" +
        String(min_lat, 6) +
        "&lomin=" + String(min_lon, 6) +
        "&lamax=" + String(max_lat, 6) +
        "&lomax=" + String(max_lon, 6);

    HTTPClient flight_http;
    http_client->setTimeout(15000);

    if (!http_client->begin(wifi_client, url)) {
        Serial.println("Failed to initialize flight request");
        return "";
    }

    http_client->addHeader(
        "Authorization",
        "Bearer " + access_token
    );

    int code = http_client->GET();

    if (code == HTTP_CODE_OK) {
        String response = http_client->getString();

        // Useful for monitoring your actual credit balance.
        String remaining =
            http_client->header("X-Rate-Limit-Remaining");

        if (!remaining.isEmpty()) {
            Serial.printf(
                "OpenSky credits remaining: %s\n",
                remaining.c_str()
            );
        }

        http_client->end();
        return response;
    }

    Serial.printf("OpenSky request failed: HTTP %d\n", code);

    if (code == 401) {
        // Token may have expired or been rejected.
        token_valid = false;
    } else if (code == 429) {
        Serial.println("Rate limit reached; back off before retrying");
        String retry_after =
            http_client->header(
                "X-Rate-Limit-Retry-After-Seconds"
            );

        if (!retry_after.isEmpty()) {
            Serial.printf(
                "Retry after %s seconds\n",
                retry_after.c_str()
            );
        }
    }

    http_client->end();
    return "";
}

std::list<Flight> FlightManager::get_flights()
{

return flights;

}

void FlightMapper::map_flights()
{
    std::list<Flight> flights = flight_manager->get_flights();
    display_manager->cls();
    display_manager->persist_scope();
    display_manager->draw_axes();

    for (const Flight& flight : flights)
    {
        float x = (flight.longitude - center_lon) * 111; // Approximate conversion to km
        float y = (flight.latitude - center_lat) * 111; // Approximate conversion to km
        int screen_x = (int)(x / RADIUS_KM * 120 + 120); // Map to screen coordinates
        int screen_y = (int)(120 - (y / RADIUS_KM * 120)); // Invert Y so higher latitude maps toward the top of the display
        display_manager->draw_flight(screen_x, screen_y, flight.callsign, flight.true_heading, flight.geo_altitude, GC9A01A_WHITE);
    }
}

String FlightManager::url_encode(const String& value)
{
    const char hex[] = "0123456789ABCDEF";
    String encoded;
    encoded.reserve(value.length() * 3);

    for (size_t i = 0; i < value.length(); i++) {
        uint8_t c = static_cast<uint8_t>(value[i]);

        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += static_cast<char>(c);
        } else {
            encoded += '%';
            encoded += hex[c >> 4];
            encoded += hex[c & 0x0F];
        }
    }

    return encoded;
}

bool FlightManager::ensure_access_token(
    WiFiClientSecure& wifi_client)
{
    // Reuse the cached token until it is close to expiring.
    if (token_valid &&
        (unsigned long)(millis() - token_fetched_at)
            < token_lifetime_ms) {
        return true;
    }

    HTTPClient token_http;
    token_http.setTimeout(10000);

    if (!token_http.begin(wifi_client, OPEN_SKY_TOKEN_URL)) {
        Serial.println("Failed to initialize OAuth request");
        return false;
    }

    token_http.addHeader(
        "Content-Type",
        "application/x-www-form-urlencoded"
    );

    String body =
        "grant_type=client_credentials"
        "&client_id=" +
        url_encode(String(OPEN_SKY_CLIENT_ID)) +
        "&client_secret=" +
        url_encode(String(OPEN_SKY_CLIENT_SECRET));

    int code = token_http.POST(body);
    String response = token_http.getString();
    token_http.end();

    if (code != HTTP_CODE_OK) {
        Serial.printf("OpenSky token request failed: %d\n", code);
        // Avoid printing the response or credentials in production.
        token_valid = false;
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error || !doc["access_token"].is<const char*>()) {
        Serial.println("Invalid OAuth token response");
        token_valid = false;
        return false;
    }

    access_token = doc["access_token"].as<String>();

    unsigned long expires_in =
        doc["expires_in"] | 1800UL;

    // Refresh 60 seconds before expiration.
    unsigned long usable_seconds =
        expires_in > 60 ? expires_in - 60 : expires_in;

    token_fetched_at = millis();
    token_lifetime_ms = usable_seconds * 1000UL;
    token_valid = !access_token.isEmpty();

    Serial.println("OpenSky access token acquired");

    return token_valid;
}