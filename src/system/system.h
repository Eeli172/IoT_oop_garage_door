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
    void setCalibStateAndSave(const calibrationState_t& state);
    void setDoorStates(const doorState_t& state);
    void setDoorStatesAndSave(const doorState_t& state);
    void setWaitingForCommand(bool waiting);
    void setRecoverFromPowerOff(bool recoverState);
    void setMotor(const std::shared_ptr<Motor>& mtr);
    void setButtons(const std::shared_ptr<Switches>& btns);

    /* SYSTEM GETTERS */
    [[nodiscard]] calibrationState_t getCalibState() const;
    [[nodiscard]] doorState_t getPreviousDoorState() const;
    [[nodiscard]] doorState_t getCurrentDoorState() const;
    [[nodiscard]] bool getWaitingForCommand() const;
    [[nodiscard]] bool getRecoverFromPowerOff() const;
    [[nodiscard]] bool getDoorErrorState() const;


    /* IO DEVICES */
    
    // MQTT
    void connectToWifi() const;
    void connectToMQTTServer() const;
    void disconnectFromMQTTServer() const;
    void subPub(
        const mqttSubState_t& transmissionType, 
        const std::string& topic,
        const std::optional<std::string>& msg
    ) const;
    bool validateDriveCommand(const std::string &rC) const;

    // EEPROM
    void saveSystem(uint16_t addr);
    void loadSystem();

    // STATUS LEDS
    void createStatusLeds();
    void updateStatusLeds() const;
    void enableStatusLeds(const bool& enbld);



private:
    /* SYSTEM VARIABLES */
    calibrationState_t calibState;
    doorState_t currentDoorState;
    doorState_t previousDoorState;
    bool waitingForCommand;
    bool recoverFromPowerOff;

    /* SYSTEM IO DEVICES */
    std::unique_ptr<MQTTManager> mqttBroker;
    std::unique_ptr<Eeprom> eeprom;
    std::shared_ptr<StatusLeds> statusLeds;
    std::shared_ptr<Motor> motor;
    std::shared_ptr<Switches> buttons;
};

#endif //GARAGE_DOOR_SYS_H
