#include "sys.h"


System::System() :
sysState(PRE_CALIB), calibrationStatus(false), doorState(CLOSED)
{}

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

stateMachine_t System::getSysState() const
{
    return this->sysState;
}


