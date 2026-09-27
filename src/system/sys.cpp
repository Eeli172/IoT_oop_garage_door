#include "sys.h"


System::System() :
sysState(PRE_CALIB), calibrationStatus(false), doorState(CLOSED)
{}


// SETTER FUNCTIONS
void System::setSysState(const stateMachine_t& state)
{
    this->sysState = state;
}

void System::setCalibrationStatus(const bool& status)
{
    this->calibrationStatus = status;
}

void System::setDoorState(const doorState_t &state)
{
    this->doorState = state;
}


// GETTER FUNCTIONS
stateMachine_t System::getSysState() const
{
    return this->sysState;
}


// MQTT FUNCTIONS
bool System::connectToMQTTServer()
{

}
