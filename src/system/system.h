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

class System : public std::enable_shared_from_this<System>
{
public:
    System();
    System(const System&) = delete;
    ~System() = default;

    // ========================================================================
    // SYSTEM SETTERS
    // ========================================================================

    void setCalibState(const calibrationState_t& state);
    void setCalibStateAndSave(const calibrationState_t& state);
    void setDoorStates(const doorState_t& state);
    void setDoorStatesAndSave(const doorState_t& state);
    void setWaitingForCommand(bool waiting);
    void setRecoverFromPowerOff(bool recoverState);
    void setMotor(const std::shared_ptr<Motor>& mtr);
    void setButtons(const std::shared_ptr<Switches>& btns);

    // ========================================================================
    // SYSTEM GETTERS
    // ========================================================================

    [[nodiscard]] calibrationState_t getCalibState() const;
    [[nodiscard]] doorState_t getPreviousDoorState() const;
    [[nodiscard]] doorState_t getCurrentDoorState() const;
    [[nodiscard]] bool getWaitingForCommand() const;
    [[nodiscard]] bool getRecoverFromPowerOff() const;
    [[nodiscard]] bool getDoorErrorState() const;

    // ========================================================================
    // IO Device Methods
    // ========================================================================

    // MQTT
    /**
     * @brief Interface method to attempt connection to a Wi-Fi via associated class 'MQTTManager'
     */
    void connectToWifi() const;

    /**
     * @brief Interface method to attempt connection to a MQTT Broker via associated class 'MQTTManager'
     */
    void connectToMQTTServer() const;

    /**
     * @brief Interface method to attempt disconnection of a connected MQTT client via associated class 'MQTTManager'
     */
    void disconnectFromMQTTServer() const;

    /**
     * @brief Interface method to handle all MQTT transmission types: Publish, Subscribe, Unsubscribe.
     * @param transmissionType Defines the transmission type, either publish, subscribe or unsubscribe.
     * @param topic Defines the target topic.
     * @param msg Optionally for publishes, an additional parameter, a message is required.
     * @return no-return but posts a descriptive message to the output stream.
     */
    void subPub(
        const mqttSubState_t& transmissionType, 
        const std::string& topic,
        const std::optional<std::string>& msg
    ) const;

    /**
     * @brief Validates the correctness of the drive command retrieved from the mqtt topic, results in motor movement.
     * @return true if command is either OPEN, CLOSE, STOP or REVERSE
     */
    bool validateDriveCommand(const std::string &rC) const;


    // EEPROM
    /**
     * @brief Saves the value of a key class attribute to the given address as parameter.
     * Additionally, data persistence and correctness is maintained with complementary storaging.
     * @param addr Address used for saving the data to the EEPROM. 16-bit / 2-byte addressing is used.
     */
    void saveSystem(uint16_t addr);

    /**
     * @brief At boot, tries to load up saved data from the EEPROM chip one attribute at a time.
     * @return no-return but method ends after a read error occurs (some data might have been retrieved).
     */
    void loadSystem();


    // STATUS LEDS
    /**
     * @brief Initiates a key IO device for physical indication of current system status.
     * A shared-weak pointer chain is formed.
     */
    void createStatusLeds();

    /**
     * @brief Updates the RP2040's LED GPIOS 20, 21, 22 according to the current door status.
     */
    void updateStatusLeds() const;

    /**
     * @brief Status leds are initiated at an earlier system stage than they are needed.
     * This is called in main.cpp just before the leds are needed in the last system state of CALIBRATED.
     */
    void enableStatusLeds(const bool& enbld);

private:

    // ========================================================================
    // System Variables
    // ========================================================================
    calibrationState_t calibState;
    doorState_t currentDoorState;
    doorState_t previousDoorState;
    bool waitingForCommand;
    bool recoverFromPowerOff;

    // ========================================================================
    // System IO Devices
    // ========================================================================
    std::unique_ptr<MQTTManager> mqttBroker;
    std::unique_ptr<Eeprom> eeprom;
    std::shared_ptr<StatusLeds> statusLeds;
    std::shared_ptr<Motor> motor;
    std::shared_ptr<Switches> buttons;
};

#endif //GARAGE_DOOR_SYS_H
