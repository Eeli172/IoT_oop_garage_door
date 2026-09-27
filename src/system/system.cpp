/* STL */
#include <iostream>

/* CUSTOM */
#include "system.h"



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
void System::connectToMQTTServer() const
{
    // [1]: Attempt to connect to a Wi-Fi.
    if (this->mqttBroker->initWiFi())
    {
        /* [2]: Create a new mqtt client instance, initiate callback functions for connection,
        subscribe & publish */
        if (this->mqttBroker->createMQTTClient())
        {
            std::cout << "MQTT-SYSTEM relationship established successfully." << std::endl;
        }
    }
}
