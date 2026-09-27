#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */

/* CUSTOM */
#include "../types/types.h"
#include "mqtt/mqtt.h"

/* SYSTEM */
// STATE MACHINE ENUM

// SYSTEM MAIN CLASS
class System
{
public:
    System();
    System(const System&) = delete;
    ~System() = default;

    void setSysState(const stateMachine_t &state);
    void setCalibrationStatus(const bool &status);
    void setDoorState(const doorState_t &state);

    stateMachine_t getSysState() const;

    // WIFI + MQTT
    bool connectToMQTTServer();

    // EEPROM
    void saveSystem();
    void loadSystem();

private:
    stateMachine_t sysState;
    bool calibrationStatus;
    doorState_t doorState;
    MQTTManager mqttBroker;
};

#endif //GARAGE_DOOR_SYS_H