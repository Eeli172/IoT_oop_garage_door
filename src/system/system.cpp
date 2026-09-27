#include "system.h"


System::System() :
doorState(CLOSED), calibState(PRE_CALIB)
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
bool System::connectToMQTTServer()
{

    return true;
}
