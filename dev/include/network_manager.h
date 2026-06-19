#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>

typedef struct
{
char            ssid[32];
char            password[64];

} network_config_t;

extern network_config_t home_network;

class NetworkManager
{

Preferences prefs;
network_config_t current_network;
WiFiClientSecure  wifi_client;

    public:

NetworkManager
                (
                );

WiFiClientSecure& get_client
                (
                );

void set
                (
                network_config_t& config
                );

bool connect
                (
                );

void dynamic_network_config
                (
                network_config_t& config
                );

network_config_t load_network_config
                (
                );

    private:

};

#endif