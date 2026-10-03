#ifndef GARAGE_DOOR_SYS_H
#define GARAGE_DOOR_SYS_H

/* STL */
#include <memory>
#include <optional>

/* CUSTOM */
#include "../types/types.h"
#include "eeprom/eeprom.h"
#include "mqtt/mqtt.h"
#include "gpios/leds/leds.h"
#include "../motor_run/motor_run.h"

/* SYSTEM */
// SYSTEM MAIN CLASS
class System : public std::enable_shared_from_this<System>
{
public:
    System();
    System(const System&) = delete;
    ~System() = default;

    /* SYSTEM SETTERS */
    void setCalibState(const calibrationState_t& state);
    void setCurrentDoorState(const doorState_t& state);
    void setWaitingForCommand(bool waiting);

    /* SYSTEM GETTERS */
    [[nodiscard]] calibrationState_t getCalibState() const;
    [[nodiscard]] bool getDoorErrorState() const;
    [[nodiscard]] doorState_t getPreviousDoorState() const;
    [[nodiscard]] doorState_t getCurrentDoorState() const;
    [[nodiscard]] bool isWaitingForCommand() const;

    /* IO DEVICES*/
    // MQTT
    void connectToMQTTServer();
    void subPub(const mqttSubState_t& transmissionType, const std::string& topic,
                const std::optional<std::string>& msg) const;
    std::string getDriveCommand();


    // EEPROM
    void saveSystem(uint16_t addr);
    void loadSystem();


    // MOTOR
    void setMotor(const std::shared_ptr<Motor>& Mtr);


    // STATUS LEDS
    void createStatusLeds();
    void updateStatusLeds() const;

private:
    /* SYSTEM STATES */
    calibrationState_t calibState;
    doorState_t currentDoorState;
    doorState_t previousDoorState;
    bool waitingForCommand = false;


    /* SYSTEM IO DEVICES */
    std::unique_ptr<MQTTManager> mqttBroker;
    std::unique_ptr<Eeprom> eeprom;
    std::shared_ptr<StatusLeds> statusLeds;
    std::shared_ptr<Motor> motor;
};

#endif //GARAGE_DOOR_SYS_H
