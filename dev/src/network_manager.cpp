#include "network_manager.h"
#include "display_manager.h"

NetworkManager::NetworkManager
                                (
                                    
                                )
{

}

WiFiClientSecure& NetworkManager::
                        get_client
                                (

                                )
{
wifi_client.setInsecure(); // Disable SSL certificate verification for simplicity, not recommended for production
return wifi_client;

}

void NetworkManager::set
                                (

                                network_config_t& config

                                )
{

current_network = config;
dynamic_network_config(config);

}

bool NetworkManager::connect
                                (

                                )
{

DisplayManager* dm = DisplayManager::getInstance();

current_network = load_network_config();

if (current_network.ssid[0] == '\0')
{
    if (dm)
    {
        dm->print("No saved network config");
        delay(2000);
        dm->cls();
    }

    return false;
}

if (dm)
{
    dm->print("Connecting to WiFi...");
    delay(1000);
    dm->cls();
}

WiFi.begin(current_network.ssid, current_network.password);

uint32_t startTime = millis();
const uint32_t timeoutMs = 30000;

while (WiFi.status() != WL_CONNECTED)
{

if (millis() - startTime > timeoutMs)
    {
    if (dm)
    {
        dm->print("WiFi Timeout");
        delay(2000);
        dm->cls();
    }

    return false;
    }

    if (dm)
        dm->initializing_display();

    delay(500);
}

if (dm)
{
    dm->cls();
    dm->print("Connected to WiFi!");
    delay(1000);
    dm->cls();
}

return true;

}

void NetworkManager::
            dynamic_network_config
                                (

                                network_config_t& config

                                )
{

prefs.begin("network", false);

prefs.putString("ssid", config.ssid);
prefs.putString("password", config.password);

prefs.end();

}

network_config_t NetworkManager::
                load_network_config
                                (

                                )
{

network_config_t config = {};

prefs.begin("network", true);

String ssid = prefs.getString("ssid", "");
String password = prefs.getString("password", "");

prefs.end();

strncpy(config.ssid,
        ssid.c_str(),
        sizeof(config.ssid) - 1);

strncpy(config.password,
        password.c_str(),
        sizeof(config.password) - 1);

config.ssid[sizeof(config.ssid) - 1] = '\0';
config.password[sizeof(config.password) - 1] = '\0';

return config;

}