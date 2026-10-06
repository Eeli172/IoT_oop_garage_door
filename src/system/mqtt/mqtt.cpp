/* STL */
#include <iostream>
#include <ostream>
#include <cstring>
#include <optional>

/* CUSTOM */
#include "mqtt.h"
#include "lwip/apps/mqtt.h"
#include "../personal_config.h"

#include "../../macros/macros.h"
#include "pico/cyw43_arch.h"
#include "types/types.h"


/* CLASS MQTTManager */
MQTTManager::MQTTManager() :
wiFiManager(WIFI()), hardwareEnabled(false), mqttEnabled(false), mqttInfo(nullptr),
currentTopic(MQTT_TOPIC_RUN_INSTRUCTIONS)
{
    this->getBrokerIpOctets();
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
        std::cout << "[WIFI-SYSTEM-STATUS] --> Wi-Fi connection successful." << std::endl;
        return true;
    }

    std::cout << "[WIFI-SYSTEM-STATUS] --> Wi-Fi connection failure." << std::endl;
    return false;
}

bool MQTTManager::establishMQTTClient()
{
    if (!mqttInfo.mqtt_client)
    {
        cyw43_arch_lwip_begin();
        mqttInfo.mqtt_client = mqtt_client_new();
        cyw43_arch_lwip_end();

        if (mqttInfo.mqtt_client)
        {
            IP_ADDR4(&mqttInfo.broker_ip, brokerIpOctets[0], brokerIpOctets[1], brokerIpOctets[2], brokerIpOctets[3]);

            memset(&mqttInfo.ci, 0, sizeof(mqttInfo.ci));

            mqttInfo.ci.client_id = MQTT_CLIENT_ID;
            mqttInfo.ci.keep_alive = MQTT_CLIENT_KA_TIME;
            mqttInfo.ci.client_pass = nullptr;
            mqttInfo.ci.client_user = nullptr;
            mqttInfo.ci.will_topic = nullptr;
        }

        else return false;
    }

    cyw43_arch_lwip_begin();
    const err_t errCon = mqtt_client_connect(
        mqttInfo.mqtt_client,
        &mqttInfo.broker_ip,
        1883,
        mqttConnectCb,
        this,
        &mqttInfo.ci
    );
    cyw43_arch_lwip_end();

    return errCon == ERR_OK;
}

bool MQTTManager::disconnectMQTTClient() const
{
    if (mqtt_client_is_connected(this->mqttInfo.mqtt_client))
    {
        mqtt_disconnect(this->mqttInfo.mqtt_client);
        return true;
    }
    return false;
}


/* UTILITY METHODS */

bool MQTTManager::getMqttStatus() const
{
    return this->mqttEnabled;
}

std::optional<std::string> MQTTManager::getNextCommand()
{
    if (!this->nextCommandBuffer.empty())
    {
        std::string tempCommandBuffer = this->nextCommandBuffer;
        this->nextCommandBuffer.clear();
        return tempCommandBuffer;
    }
    return std::nullopt;
}

void MQTTManager::setTopic(const std::string& newTopic)
{
    if (!this->currentTopic.empty())
    {
        this->previousTopic = this->currentTopic;
    }

    this->currentTopic = newTopic;
}

void MQTTManager::mqttIncomingPublishCb(void *arg, const char *topic, u32_t tot_len)
{
   printf("[MQTT-DATA-CB] --> Incoming message with length of: %d, from topic: %s\n", tot_len, topic);
}

void MQTTManager::mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        self->nextCommandBuffer.append(reinterpret_cast<const char*>(data), len);

        printf("[MQTT-DATA-CB] --> Data received: %s\n", self->nextCommandBuffer.c_str());

        if (flags & MQTT_DATA_FLAG_LAST)
        {
            printf("[MQTT-DATA-CB] --> Full message payload received.\n");
        }
    }
}



void MQTTManager::mqttSubscribeCb(void* arg, err_t result)
{
    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        if (result != ERR_OK)
        {
            printf("[MQTT-SUBSCRIBE-ERROR-CB] --> MQTT subscribe failed with result: %d\n", result);
            return;
        }

        printf("[MQTT-SUBSCRIBE-CB] --> MQTT subscribe successful.\n");
    }
}

void MQTTManager::mqttPublishCb(void* arg, err_t result)
{
    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        if (result != ERR_OK)
        {
            printf("[MQTT-PUBLISH-ERROR-CB] --> MQTT publish failed with status: %d\n", result);
            return;
        }

        printf("[MQTT-PUBLISH-CB] --> MQTT publish successful.\n");
    }
}

/* MQTT CONNECTION SETUP */
void MQTTManager::mqttConnectCb(mqtt_client_t* client, void *arg, mqtt_connection_status_t status)
{
    if (status != MQTT_CONNECT_ACCEPTED && status != MQTT_CONNECT_TIMEOUT && status != MQTT_CONNECT_DISCONNECTED)
    {
        printf("[MQTT-CONNECT-ERROR-CB] --> Connection failed with status: %d\n", status);
        return;
    }

    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        if (status == MQTT_CONNECT_ACCEPTED)
        {
            printf("[MQTT-CONNECT-CB] --> MQTT Connection successful.\n");
            self->mqttEnabled = true;

            // [1]. Set callbacks for incoming publishes and data handling from a subscribed topic
            mqtt_set_inpub_callback(client,
                mqttIncomingPublishCb,
                mqttIncomingDataCb,
                self);

            self->setTopic(MQTT_TOPIC_RUN_INSTRUCTIONS);
            self->subscribeToTopic(SUBSCRIBE);
        }

        else
        {
            self->mqttEnabled = false;
            printf("[MQTT-CONNECT-ERROR-CB] --> Connection disconnected, attempting reconnection in a moment, status: %d\n", status);
        }
    }
}

// MQTT SUBSCRIBE FUNCTION
bool MQTTManager::subscribeToTopic(const mqttSubState_t &subsState)
{
    cyw43_arch_lwip_begin();
    if (!mqtt_client_is_connected(this->mqttInfo.mqtt_client))
    {
        cyw43_arch_lwip_end();
        return false;
    }

    const err_t errSub = mqtt_sub_unsub(mqttInfo.mqtt_client, this->currentTopic.c_str(), 1, mqttSubscribeCb, this, subsState);
    cyw43_arch_lwip_end();

    if (errSub != ERR_OK)
    {
        std::cout << "[MQTT-SUBSCRIBE-ERROR] --> Error whilst trying to use \"mqtt_sub_unsub()\" function." << std::endl;
        return false;
    }

    return true;
}

// MQTT PUBLISH FUNCTION
bool MQTTManager::publishToTopic(const std::string &msg)
{
    cyw43_arch_lwip_begin();
    if (!mqtt_client_is_connected(this->mqttInfo.mqtt_client))
    {
        cyw43_arch_lwip_end();
        return false;
    }

    const err_t errSub = mqtt_publish(mqttInfo.mqtt_client, this->currentTopic.c_str(),
        msg.c_str(), msg.length(),
        1, 0, mqttPublishCb, this);
    cyw43_arch_lwip_end();

    if (errSub != ERR_OK)
    {
        std::cout << "[MQTT-PUBLISH-ERROR] --> Error whilst trying to use \"mqtt_publish()\" function: " << errSub << std::endl;
        return false;
    }

    return true;
}


void MQTTManager::getBrokerIpOctets() const {
    const char* ipStr = MQTT_BROKER_IP;
    sscanf(ipStr, "%d.%d.%d.%d", 
        &this->brokerIpOctets[0], 
        &this->brokerIpOctets[1], 
        &this->brokerIpOctets[2], 
        &this->brokerIpOctets[3]
    );
}