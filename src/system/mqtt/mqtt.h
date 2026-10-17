#ifndef IOT_OOP_GARAGE_DOOR_MQTT_H
#define IOT_OOP_GARAGE_DOOR_MQTT_H

/* STL */
#include <array>
#include <optional>
#include <string>

/* CUSTOM HEADERS */
#include "../../macros/macros.h"
#include "../wifi/wifi.h"
#include "types/types.h"

class MQTTManager {
public:
    MQTTManager();

    // ========================================================================
    // Connection Management
    // ========================================================================

    /**
     * @brief Initializes hardware (cyw43) and attempts connection to Wi-Fi with user specified params in personal_config.h file.
     * @return true if both hardware initialization and Wi-Fi connection succeed, false otherwise.
     */
    bool initWiFi();

    /**
     * @brief Creates MQTT client, initializes client information, and attempts connection to a broker.
     * @return true if instance creation and connection attempt were successful, false otherwise.
     */
    bool establishMQTTClient();

    /**
     * @brief Checks MQTT connection status and disconnects the client if currently connected.
     * @return true if MQTT client was disconnected successfully, false otherwise.
     */
    [[nodiscard]] bool disconnectMQTTClient();

    // ========================================================================
    // Getters & Status
    // ========================================================================

    /**
     * @brief Returns the current connection status of the MQTT client.
     * @return true if connected to the broker, false otherwise.
     */
    [[nodiscard]] bool getMqttStatus() const;

    /**
     * @brief Checks for a pending command received from an MQTT topic.
     * @return The next command string if present, std::nullopt if no new commands exists.
     */
    [[nodiscard]] std::optional<std::string> getNextCommand();

    /**
     * @brief Parses the broker IP address defined in personal_config.h into octets.
     */
    void getBrokerIpOctets() const;

    // ========================================================================
    // Messaging (Subscribe & Publish)
    // ========================================================================

    /**
     * @brief Subscribes to or unsubscribes from the given topic.
     * @param topic Target MQTT topic. Cannot be empty.
     * @param subsState Subscription type (e.g., subscribe or unsubscribe).
     * @return true if the transmission was attempted successfully, false otherwise.
     */
    bool subscribeToTopic(const std::string &topic, const mqttSubState_t &subsState);

    /**
     * @brief Publishes a message payload to the specified topic.
     * @param topic Destination MQTT topic. Cannot be empty.
     * @param msg Payload message string to be sent.
     * @return true if publishing attempt was successful, false otherwise.
     */
    bool publishToTopic(const std::string &topic, const std::string &msg);

    // ========================================================================
    // MQTT Callbacks (lwIP / SDK)
    // ========================================================================

    /** @brief Callback for MQTT connection events. */
    static void mqttConnectCb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

    /** @brief Callback for MQTT subscription operation results. */
    static void mqttSubscribeCb(void *arg, err_t result);

    /** @brief Callback triggered when an incoming topic header is received. */
    static void mqttIncomingPublishCb(void *arg, const char *topic, u32_t tot_len);

    /** @brief Callback triggered when incoming topic payload data arrives. */
    static void mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags);

    /** @brief Callback for MQTT publish operation results. */
    static void mqttPublishCb(void *arg, err_t result);

private:
    WIFI wiFiManager;
    bool hardwareEnabled;

    std::array<int, IP_OCTETS> brokerIpOctets;
    bool mqttEnabled;
    mqttInformation_t mqttInfo;

    std::string nextCommandBuffer;
};

#endif // IOT_OOP_GARAGE_DOOR_MQTT_H