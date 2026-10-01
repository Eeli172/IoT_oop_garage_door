/* STL */
#include <iostream>

/* CUSTOM */
#include "system.h"

#include "pico/cyw43_arch.h"
#include "pico/time.h"


System::System() :
doorState(CLOSED), calibState(PRE_CALIB), mqttBroker(std::make_unique<MQTTManager>())
{}


// SETTER FUNCTIONS
void System::setCalibState(const calibrationState_t& state)
{
    this->calibState = state;
}


void System::setDoorState(const doorState_t &state)
{
    this->doorState = state;
}


// GETTER FUNCTIONS
calibrationState_t System::getCalibState() const
{
    return this->calibState;
}


// MQTT FUNCTIONS
void System::connectToMQTTServer()
{
    // [1]: Attempt to connect to a Wi-Fi.
    if (this->mqttBroker->initWiFi())
    {
        /* [2]: Create a new mqtt client instance, initiate callback functions for connection,
        subscribe & publish */
        if (this->mqttBroker->establishMQTTClient())
        {
            const absolute_time_t timeout = get_absolute_time();
            while (!this->mqttBroker->getMqttStatus() && absolute_time_diff_us(timeout, get_absolute_time()) < 10000000)
            {
                cyw43_arch_poll();
                sleep_ms(1);
            }

            if (this->mqttBroker->getMqttStatus())
            {
                std::cout << "MQTT connection established successfully" << std::endl;
            }

            else
            {
                std::cout << "MQTT connection ran into a timeout." << std::endl;
            }
        }
    }
}

void System::subPub(const mqttSubState_t &transmissionType, const std::string &topic, const std::optional<std::string> &msg) const
{
    if (!this->mqttBroker->getMqttStatus()) return;

    std::string errorMsg;
    if (transmissionType == UNSUBSCRIBE || transmissionType == SUBSCRIBE)
    {
            this->mqttBroker->setTopic(topic);
            this->mqttBroker->subscribeToTopic(transmissionType); // tää pitäs laittaa iffin sisää tai jotenkin tarkastaa
    }

    else
    {
        if (msg.has_value())
        {
            this->mqttBroker->setTopic(topic);
            this->mqttBroker->publishToTopic(msg.value());
        }
        else
        {
            errorMsg = "[MQTT-SUBPUB] --> msg had no value, transmission type: <PUBLISH>";
        }
    }

    if (!errorMsg.empty())
    {
        std::cout << errorMsg << std::endl;
        return;
    }

    const absolute_time_t timeout = get_absolute_time();
    while (!this->mqttBroker->getMqttTransmissionStatus() && absolute_time_diff_us(timeout, get_absolute_time()) < 10000000)
    {
        cyw43_arch_poll();
        sleep_ms(1);
    }

    if (this->mqttBroker->getMqttTransmissionStatus())
    {
        this->mqttBroker->reverseMqttTransmissionStatus();
    }
}

std::string System::getDriveCommand()
{
    return this->mqttBroker->getNextCommand();
}
