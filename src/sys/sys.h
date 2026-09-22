#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */
#include <any>

/* CUSTOM */
#include "../motor_run/motor_run.h"

/* SYSTEM */
// STATE MACHINE ENUM
typedef enum { PRE_CALIB, CALIBRATING, CALIBRATED } stateMachine_t;

// SYSTEM MAIN CLASS
class sys
{
public:
    sys();
    sys(const stateMachine_t &initSysState, bool &calibrationStatus, int &initEncoderPos, doorState_t &initDoorState);
    sys(const sys&) = delete;
    ~sys() = default;

    void setCommonValue(const std::any &value);
    void getCommonValue(std::string key="default");
    void saveEEPROM();
    void loadEEPROM();

private:
    stateMachine_t sysState;
    bool calibrationStatus;
    int encoderPos;
    doorState_t doorState;
};

#endif //GARAGE_DOOR_SYS_H