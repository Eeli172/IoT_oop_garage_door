#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */
#include <memory>
#include <optional>

/* CUSTOM */
#include "../types/types.h"
#include "eeprom/eeprom.h"
#include "mqtt/mqtt.h"
#include "../motor_run/motor_run.h"

/* SYSTEM */
// SYSTEM MAIN CLASS
class System
{
public:
    System();
    System(const System&) = delete;
    ~System() = default;

    void setCalibState(const calibrationState_t &state);

    [[nodiscard]] calibrationState_t getCalibState() const;

    // MQTT
    void connectToMQTTServer();
    void subPub(const mqttSubState_t &transmissionType, const std::string &topic, const std::optional<std::string> &msg) const;
    std::string getDriveCommand();

    // EEPROM
    void saveSystem(uint16_t addr);
    void loadSystem();

    // ADD MOTOR
    void addMotor(std::shared_ptr<Motor> mtr);

private:
    doorState_t doorState;
    calibrationState_t calibState;
    std::unique_ptr<MQTTManager> mqttBroker;
    std::unique_ptr<Eeprom> eeprom;
    std::shared_ptr<Motor> motor;
};

#endif //GARAGE_DOOR_SYS_H