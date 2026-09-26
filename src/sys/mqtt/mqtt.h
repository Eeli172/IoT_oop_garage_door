#ifndef IOT_OOP_GARAGE_DOOR_MQTT_H
#define IOT_OOP_GARAGE_DOOR_MQTT_H

#include "lwip/apps/mqtt.h"
#include "wifi/wifi.h"
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

    typedef struct
    {
        mqtt_client_t *mqtt_client;
        ip_addr_t broker_ip;
        u_int16_t mqttPort;
        mqtt_connect_client_info_t ci;
    } mqtt_information;
    mqtt_information mqttInfo;

    // DATA
    std::string nextCommandBuffer;
    std::string previousTopic;
    std::string currentTopic;
    std::string message;

};

#endif //IOT_OOP_GARAGE_DOOR_MQTT_H