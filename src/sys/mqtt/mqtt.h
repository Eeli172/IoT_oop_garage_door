#ifndef IOT_OOP_GARAGE_DOOR_MQTT_H
#define IOT_OOP_GARAGE_DOOR_MQTT_H

#include "../wifi/wifi.h"
#include <string>

class MQTTManager : public WIFI
{
    public:
    MQTTManager();

    bool initWiFi();
    bool subsribeToTopic(char * topic);
    bool publishToTopic(char * topic);

    private:
    // WIFI
    WIFI wiFiManager;
    bool wiFiEnabled;

    // MQTT

};

#endif //IOT_OOP_GARAGE_DOOR_MQTT_H