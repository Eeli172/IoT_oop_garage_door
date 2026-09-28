#ifndef IOT_OOP_GARAGE_DOOR_MQTT_H
#define IOT_OOP_GARAGE_DOOR_MQTT_H

#include "../wifi/wifi.h"
#include "types/types.h"
#include <string>

class MQTTManager
{
    public:
    MQTTManager();

    /* WIFI METHODS */
    bool initWiFi();

    /* MQTT METHODS */
    bool createMQTTClient();
    static void mqttConnectCb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
    static void mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags);
    bool getMQTTStatus();

    // SUBSCRIBING TO TOPIC
    bool subscribeToTopic(const mqttSubState_t &subsState);
    static void mqttSubscribeCb(void *arg, err_t result);

    // PUBLISHING DATA TO TOPIC
    bool publishToTopic(const std::string &topic);
    static void mqttIncomingPublishCb(void *arg, const char *topic, u32_t tot_len);
    static void mqttPublishCb(void *arg, err_t result);
    void setTopic(const std::string &newTopic);

    private:
    /* WIFI + HARDWARE ATTRIBUTES */
    WIFI wiFiManager;
    bool hardwareEnabled;

    /* MQTT ATTRIBUTES */
    bool mqttEnabled;
    mqtt_information mqttInfo;

    // DATA
    std::string nextCommandBuffer; // Holds the next command in line { RUN, STOP } .
    std::string previousTopic; // Once a new topic is subscribed to, holds the info on which topic to first unsubscribe.
    std::string currentTopic; // Holds the newest and currently subscribed topic.
    std::string message; // String to hold the status-message to publish to the currently subscribed topic.

};

#endif //IOT_OOP_GARAGE_DOOR_MQTT_H