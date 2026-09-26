#ifndef IOT_OOP_GARAGE_DOOR_CONFIG_H
#define IOT_OOP_GARAGE_DOOR_CONFIG_H

// 1. Copy this file as "config.h" (included in .gitignore)
// 2. Fill the config 

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define WIFI_CONNECTING_TIMEOUT_MS 5000

#define MQTT_BROKER_IP ""
#define MQTT_BROKER_PORT 1883

#define MQTT_CLIENT_ID "RP2040"
#define MQTT_CLIENT_KA_TIME 60
#define MQTT_DEFAULT_TOPIC "garage_door/status"
#define MQTT_DEFAULT_MSG "<Initialization complete>"


#endif //IOT_OOP_GARAGE_DOOR_CONFIG_H