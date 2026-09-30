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
        if (this->mqttBroker->createMQTTClient())
        {
            const absolute_time_t timeout = get_absolute_time();
            while (!this->mqttBroker->getMQTTStatus() && absolute_time_diff_us(timeout, get_absolute_time()) < 30000000)
            {
                cyw43_arch_poll();
                
                cyw43_arch_lwip_begin();
                
                cyw43_arch_lwip_end();
                
                sleep_ms(10);
            }

            if (this->mqttBroker->getMQTTStatus())
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

void System::subscribeToTopic(const std::string &topic) const
{
    this->mqttBroker->setTopic(topic);
    if (this->mqttBroker->subscribeToTopic(SUBSCRIBE))
    {
        std::cout << "MQTT subscription established successfully to topic: " << topic << std::endl;
    }
}
