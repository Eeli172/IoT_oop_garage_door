/* STL */
#include <iostream>
#include <ostream>
#include <cstring>

/* CUSTOM */
#include "mqtt.h"
#include "lwip/apps/mqtt.h"
#include "../personal_config.h"

#include "../../macros/macros.h"
#include "pico/cyw43_arch.h"
#include "types/types.h"


/* CLASS MQTTManager */
MQTTManager::MQTTManager() :
wiFiManager(WIFI()), hardwareEnabled(false), mqttEnabled(false),
currentTopic(MQTT_DEFAULT_TOPIC), message(MQTT_DEFAULT_MSG)
{}

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

bool MQTTManager::createMQTTClient()
{
    IP_ADDR4(&mqttInfo.broker_ip, IP_A, IP_B, IP_C, IP_D);

    cyw43_arch_lwip_begin(); // mutex lukko
    mqttInfo.mqtt_client = mqtt_client_new();
    cyw43_arch_lwip_end();

    if (mqttInfo.mqtt_client == nullptr) return false;

    memset(&mqttInfo.ci, 0, sizeof(mqttInfo.ci));

    mqttInfo.ci.client_id = MQTT_CLIENT_ID;
    mqttInfo.ci.keep_alive = MQTT_CLIENT_KA_TIME;
    mqttInfo.ci.client_pass = nullptr;
    mqttInfo.ci.client_user = nullptr;
    mqttInfo.ci.will_topic = nullptr;
    mqttInfo.ci.will_msg = nullptr;
    mqttInfo.ci.will_msg_len = 0;
    mqttInfo.ci.will_qos = 0;
    mqttInfo.ci.will_retain = 0;


    cyw43_arch_lwip_begin(); // voitas tehä myös lock_guardilla = luokka, jonka constructor kutsuu tämän metodin, destructor kutsuu .end()
    const err_t errCon = mqtt_client_connect(
        mqttInfo.mqtt_client,
        &mqttInfo.broker_ip,
        1883,
        mqttConnectCb,
        this,
        &mqttInfo.ci
    );
    cyw43_arch_lwip_end();

    printf("errcon: %d\n", errCon);
    return errCon == ERR_OK;
}

bool MQTTManager::getMQTTStatus()
{
    return this->mqttEnabled;
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
   printf("[MQTT-DATA-CB] --> Incoming message with length of: %d, from topic: %s\n", tot_len, topic);
}

void MQTTManager::mqttIncomingDataCb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        const std::string dataBuffer(reinterpret_cast<const char*>(data), len);
        self->nextCommandBuffer = dataBuffer;

        printf("[MQTT-DATA-CB] --> Data received: %s\n", self->nextCommandBuffer.c_str());
    }
}

void MQTTManager::mqttSubscribeCb(void* arg, err_t result)
{
    if (result == ERR_OK)
    {
        printf("[MQTT-SUBSCRIBE-CB] --> MQTT subscribe successful.\n");
    }
    else
    {
        printf("[MQTT-SUBSCRIBE-ERROR-CB] --> MQTT subscribe failed with result: %d\n", result);
    }
}

void MQTTManager::mqttPublishCb(void* arg, err_t result)
{
    if (result == ERR_OK)
    {
       printf("[MQTT-PUBLISH-CB] --> MQTT publish successful.\n");
    }
    else
    {
        printf("[MQTT-PUBLISH-ERROR-CB] --> MQTT publish failed with status: %d\n", result);
    }
}

/* MQTT CONNECTION SETUP */
void MQTTManager::mqttConnectCb(mqtt_client_t* client, void *arg, mqtt_connection_status_t status)
{
    printf("CB called with status: %d\n", status);
    if (status != MQTT_CONNECT_ACCEPTED)
    {
        printf("[MQTT-CONNECT-ERROR-CB] --> Connection failed with status: %d\n", status);
        return;
    }

    if (auto *self = static_cast<MQTTManager*>(arg); self)
    {
        mqtt_set_inpub_callback(self->mqttInfo.mqtt_client,
            mqttIncomingPublishCb,
            mqttIncomingDataCb,
            self);
        self->mqttEnabled = true;
    }
}

// MQTT SUBSCRIBE FUNCTION
bool MQTTManager::subscribeToTopic(const mqttSubState_t &subsState)
{
    if (!mqttEnabled) return false;

    cyw43_arch_lwip_begin();
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
bool MQTTManager::publishToTopic(const std::string &topic)
{
    if (!mqttEnabled) return false;

    cyw43_arch_lwip_begin();
    const err_t errSub = mqtt_publish(mqttInfo.mqtt_client, topic.c_str(),
        message.c_str(), message.length(),
        1, 0, mqttPublishCb, this);
    cyw43_arch_lwip_end();

    if (errSub != ERR_OK)
    {
        std::cout << "[MQTT-PUBLISH-ERROR] --> Error whilst trying to use \"mqtt_publish()\" function: " << errSub << std::endl;
        return false;
    }

    return true;
}