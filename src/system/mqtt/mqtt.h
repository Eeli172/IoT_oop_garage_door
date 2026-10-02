#ifndef IOT_OOP_GARAGE_DOOR_MQTT_H
#define IOT_OOP_GARAGE_DOOR_MQTT_H

#include "../wifi/wifi.h"
#include "types/types.h"
#include <string>
#include <array>

class MQTTManager
{
    public:
    MQTTManager();

    /* WIFI METHODS */
    bool initWiFi();

    /* MQTT METHODS */

    // ESTABLISHING A CLIENT INSTANCE + CALLBACK FOR SUCCESSFUL CONNECTION TO BROKER
    bool establishMQTTClient();
    static void mqttConnectCb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

    // SUBSCRIBING TO TOPIC
    bool subscribeToTopic(const mqttSubState_t &subsState);
    static void mqttSubscribeCb(void *arg, err_t result);

    // RECEIVING DATA FROM SUBSCRIBED TOPIC
    static void mqttIncomingPublishCb(void *arg, const char *topic, u32_t tot_len);
    static void mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags);

    // PUBLISHING DATA TO TOPIC
    bool publishToTopic(const std::string &msg);
    static void mqttPublishCb(void *arg, err_t result);
    void setTopic(const std::string &newTopic);

    // UTILITY FUNCTIONS
    bool getMqttStatus() const; // Determines the connection to the MQTT broker.
    bool getMqttTransmissionStatus() const; // Determines whether an interaction (PUB, SUB, UNSUB) was successful with the broker
    void reverseMqttTransmissionStatus(); // After a positive callback is received, the function is called to toggle the status.
    std::string getNextCommand();

    private:
    void getBrokerIpOctets() const;
    /* WIFI + HARDWARE ATTRIBUTES */
    WIFI wiFiManager;
    bool hardwareEnabled;

    /* MQTT ATTRIBUTES */
    std::array<int, IP_OCTETS> brokerIpOctets;
    bool mqttEnabled;
    bool mqttSubPubStatus;

    mqtt_information mqttInfo;

    // DATA
    std::string nextCommandBuffer; // Holds the next command in line { RUN, STOP } .
    std::string previousTopic; // Once a new topic is subscribed to, holds the info on which topic to first unsubscribe.
    std::string currentTopic; // Holds the newest and currently subscribed topic.

};

#endif //IOT_OOP_GARAGE_DOOR_MQTT_H