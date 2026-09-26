/* STL */
#include <iostream>
#include <ostream>
#include <cstring>

/* CUSTOM */
#include "mqtt.h"
#include "lwip/apps/mqtt.h"

#include "config.h"
#include "types/types.h"


/* CLASS MQTTManager */
MQTTManager::MQTTManager() :
wiFiManager(WIFI()), hardwareEnabled(false), mqttEnabled(false),
currentTopic(MQTT_DEFAULT_TOPIC), message(MQTT_DEFAULT_MSG)
{}

bool MQTTManager::createMQTTClient()
{
    IP_ADDR4(&mqttInfo.broker_ip, 127, 0, 0, 1);
    mqttInfo.mqtt_client = mqtt_client_new();

    if (mqttInfo.mqtt_client != nullptr)
    {
        mqttInfo.ci.client_id = MQTT_CLIENT_ID;
        mqttInfo.ci.keep_alive = MQTT_CLIENT_KA_TIME;

        mqtt_client_connect(mqttInfo.mqtt_client, &mqttInfo.broker_ip,
            MQTT_BROKER_PORT, mqttConnectCb, this, &mqttInfo.ci);

        std::cout << "[MQTT-OK] --> MQTT Client established with the given settings" << std::endl;
        this->mqttEnabled = true;
        return true;
    }

    std::cout << "[MQTT-FAIL] --> MQTT Client establishing failure." << std::endl;
    return false;
}

bool MQTTManager::initWiFi()
{
    if (!this->hardwareEnabled)
    {
        if (this->wiFiManager.init())
        {
            std::cout << "[OK] --> Wi-Fi Hardware initialization successful." << std::endl;
            this->hardwareEnabled = true;
        }
        else
        {
            std::cout << "[FAIL] --> Wi-Fi Hardware initialization failed." << std::endl;
            return false;
        }
    }

    if (this->wiFiManager.connectWifi())
    {
        std::cout << "[OK] --> Wi-Fi connection successful." << std::endl;
        return true;
    }

    std::cout << "[ERROR] --> Wi-Fi connection failed." << std::endl;
    return false;
}

void MQTTManager::setTopic(const std::string& newTopic)
{
    if (!newTopic.empty())
    {
        this->previousTopic = this->currentTopic;
        this->currentTopic = newTopic;
    }
}

void MQTTManager::mqttIncomingPublishCb(void *arg, const char *topic, u32_t tot_len)
{
    if (auto *self = static_cast<MQTTManager*>(arg); self) {
        self->currentTopic = topic;
    }
}

void MQTTManager::mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        const std::string dataBuffer(reinterpret_cast<const char*>(data), len);
        self->nextCommandBuffer = dataBuffer;

        std::cout << "[MQTT-DATA] --> Data received: " << dataBuffer << std::endl;
    }
}

void MQTTManager::mqttSubscribeCb(void* arg, err_t result)
{
    if (result == ERR_OK)
    {
        std::cout << "[MQTT-SUBSCRIBE] --> MQTT subscribe successful." << std::endl;
    }
    else
    {
        std::cout << "[MQTT-ERROR] --> MQTT subscribe failed: " << result << std::endl;
    }
}

void MQTTManager::mqttPublishCb(void* arg, err_t result)
{
    if (result == ERR_OK)
    {
        std::cout << "[MQTT-PUBLISH] --> MQTT publish successful." << std::endl;
    }
    else
    {
        std::cout << "[MQTT-ERROR] --> MQTT publish failed: " << result << std::endl;
    }
}

/* MQTT CONNECTION SETUP */
void MQTTManager::mqttConnectCb(mqtt_client_t* client, void *arg, mqtt_connection_status_t status)
{
    if (status != MQTT_CONNECT_ACCEPTED)
    {
        std::cout << "[MQTT-ERROR] --> Connection failed with status: " << status << std::endl;
        return;
    }

    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        mqtt_set_inpub_callback(self->mqttInfo.mqtt_client,
            mqttIncomingPublishCb,
            mqttIncomingDataCb,
            self);

        // SUBSCRIBING CALLBACK
        err_t errSub = mqtt_sub_unsub(self->mqttInfo.mqtt_client, MQTT_DEFAULT_TOPIC, 0, mqttSubscribeCb, self, SUBSCRIBE);

        if (errSub != ERR_OK)
        {
            std::cout << "[MQTT-ERROR] --> Error whilst trying to use \"mqtt_sub_unsub()\" function." << std::endl;
            return;
        }

        // PUBLISHING CALLBACK
        errSub = mqtt_publish(self->mqttInfo.mqtt_client, MQTT_DEFAULT_TOPIC,
            MQTT_DEFAULT_MSG, strlen(MQTT_DEFAULT_MSG),
            0, 0, mqttPublishCb, self);

        if (errSub != ERR_OK)
        {
            std::cout << "[MQTT-ERROR] --> Error whilst trying to use \"mqtt_publish()\" function." << std::endl;
            return;
        }

        std::cout << "[MQTT-CONNECTION-OK] --> MQTT connection successful." << std::endl;
    }
}

// MQTT SUBSCRIBE FUNCTION
bool MQTTManager::subscribeToTopic(const mqttSubState_t &subsState)
{
    if (!mqttEnabled) return false;

    const err_t errSub = mqtt_sub_unsub(mqttInfo.mqtt_client, this->currentTopic.c_str(), 0, mqttSubscribeCb, this, subsState);

    if (errSub != ERR_OK)
    {
        std::cout << "[MQTT-ERROR] --> Error whilst trying to use \"mqtt_sub_unsub()\" function." << std::endl;
        return false;
    }

    return true;
}

// MQTT PUBLISH FUNCTION
bool MQTTManager::publishToTopic(const std::string &topic)
{
    if (!mqttEnabled) return false;

    const err_t errSub = mqtt_publish(mqttInfo.mqtt_client, topic.c_str(),
        message.c_str(), message.length(),
        0, 0, mqttPublishCb, this);

    if (errSub != ERR_OK)
    {
        std::cout << "[MQTT-ERROR] --> Error whilst trying to use \"mqtt_publish()\" function." << std::endl;
        return false;
    }

    return true;
}