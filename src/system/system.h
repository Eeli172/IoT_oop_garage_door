#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */
#include <memory>

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

    [[nodiscard]] calibrationState_t getCalibState() const;

    // WIFI + MQTT
    void connectToMQTTServer() const;

    // EEPROM
    void saveSystem();
    void loadSystem();

private:
    doorState_t doorState;
    calibrationState_t calibState;
    std::unique_ptr<MQTTManager> mqttBroker;
};

#endif //GARAGE_DOOR_SYS_H