#ifndef IOT_OOP_GARAGE_DOOR_MQTT_H
#define IOT_OOP_GARAGE_DOOR_MQTT_H

/* STL */
#include <string>
#include <array>
#include <optional>

/* CUSTOM HEADERS */
#include "../../macros/macros.h"
#include "../wifi/wifi.h"
#include "types/types.h"

class MQTTManager
{
    public:
    MQTTManager();

    /* WIFI METHODS */
    bool initWiFi();

    /* MQTT METHODS */
    // CLIENT INSTANCE CREATION, DISCONNECTION, CALLBACK FOR CONNECTION MANAGEMENT
    bool establishMQTTClient();
    [[nodiscard]] bool disconnectMQTTClient();
    static void mqttConnectCb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

    // UTILITY FUNCTIONS
    [[nodiscard]] bool getMqttStatus() const; // Determines the connection to the MQTT broker.
    [[nodiscard]] std::optional<std::string> getNextCommand() const;

    // SUBSCRIBING TO TOPIC + CALLBACK FOR SUCCESSFUL SUBSCRIBE
    bool subscribeToTopic(const mqttSubState_t &subsState);
    static void mqttSubscribeCb(void *arg, err_t result);

    // RECEIVING DATA FROM SUBSCRIBED TOPIC
    static void mqttIncomingPublishCb(void *arg, const char *topic, u32_t tot_len);
    static void mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags);

    // PUBLISHING DATA TO TOPIC
    bool publishToTopic(const std::string &msg);
    static void mqttPublishCb(void *arg, err_t result);
    void setTopic(const std::string &newTopic);

    private:
    void getBrokerIpOctets() const;

    /* WIFI + HARDWARE ATTRIBUTES */
    WIFI wiFiManager;
    bool hardwareEnabled;

    /* MQTT ATTRIBUTES */
    std::array<int, IP_OCTETS> brokerIpOctets;
    bool mqttEnabled; // Defines the state of connection between client and a broker.
    mqttInformation_t mqttInfo; // Struct for holding client instance information.

    // DATA
    std::string nextCommandBuffer; // Holds the next command in line { RUN, STOP } .
    std::string previousTopic; // Once a new topic is subscribed to, holds the info on which topic to first unsubscribe.
    std::string currentTopic; // Holds the newest and currently subscribed topic.

};

#endif //IOT_OOP_GARAGE_DOOR_MQTT_H