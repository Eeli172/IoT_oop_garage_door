#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */

/* CUSTOM */
#include "../types/types.h"
#include "mqtt/mqtt.h"

/* SYSTEM */
// SYSTEM MAIN CLASS
class System
{
public:
    System();
    System(const System&) = delete;
    ~System() = default;

    void setCalibState(const calibrationState_t &state);
    void setDoorState(const doorState_t &state);

    calibrationState_t getCalibState() const;

    // WIFI + MQTT
    bool connectToMQTTServer();

    // EEPROM
    void saveSystem();
    void loadSystem();

private:
    doorState_t doorState;
    calibrationState_t calibState;
    MQTTManager mqttBroker;
};

#endif //GARAGE_DOOR_SYS_H