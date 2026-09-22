#include "sys.h"
#include <any>
#include <iostream>


sys::sys() :
sysState(PRE_CALIB), calibrationStatus(false), encoderPos(0), doorState(CLOSED)
{}
void sys::setCommonValue(const std::any &value)
{
    if (const auto *systemState = std::any_cast<stateMachine_t>(&value))
    {
        this->sysState = *systemState;
    }

    else if (const auto *calibSt = std::any_cast<bool>(&value))
    {
        this->calibrationStatus = *calibSt;
    }

    else if (const auto *EncPos = std::any_cast<int>(&value))
    {
        this->encoderPos = *EncPos;
    }

    else if (const auto *DoorSt = std::any_cast<doorState_t>(&value))
    {
        this->doorState = *DoorSt;
    }

    else
    {
        std::cerr << "[FAIL] --> System common value set fail. " << std::endl;
    }
}