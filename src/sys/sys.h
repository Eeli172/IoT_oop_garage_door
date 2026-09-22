#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */

/* CUSTOM */
#include "../motor_run/motor_run.h"

/* SYSTEM */
// STATE MACHINE ENUM
typedef enum { PRE_CALIB, CALIBRATING, CALIBRATED } stateMachine_t;

// SYSTEM MAIN CLASS
class Sys
{
public:
    Sys();
    Sys(const Sys&) = delete;
    ~Sys() = default;

    void setSysState(stateMachine_t &state);
    void setCalibrationStatus(bool &status);
    void setEncoderPos(int &pos);
    void setDoorState(doorState_t &state);

    void saveSystem();
    void loadSystem();

private:
    stateMachine_t sysState;
    bool calibrationStatus;
    int encoderPos;
    doorState_t doorState;
};

#endif //GARAGE_DOOR_SYS_H