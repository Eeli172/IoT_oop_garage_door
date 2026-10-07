/* STL */
#include <iostream>
#include <memory>
#include <ostream>

/* CUSTOM */
#include "system.h"
#include "eeprom/eeprom.h"
#include "gpios/leds/leds.h"

#include "types/types.h"


/* ########## P U B L I C ########## */

System::System() :
calibState(NOT_CALIBRATED),
currentDoorState(CLOSED),
previousDoorState(CLOSED),
waitingForCommand(false),
recoverFromPowerOff(false),
mqttBroker(std::make_unique<MQTTManager>()),
eeprom(std::make_unique<Eeprom>())
{}


/* SYSTEM SETTERS */

void System::setCalibState(const calibrationState_t& state)
{
    this->calibState = state;
    if (state == CALIBRATED) {
        this->motor->setCalibState(true);
    } 
    else if (state == NOT_CALIBRATED) {
        this->motor->setCalibState(false);
    }
    std::cout << "CALIB STATE SET: " << state << std::endl;
}

void System::setCalibStateAndSave(const calibrationState_t& state)
{
    this->setCalibState(state);
    this->saveSystem(EEPROM_ADDR_CALIBRATION_STATE);
}

void System::setDoorStates(const doorState_t& state)
{
    this->previousDoorState = this->currentDoorState;
    this->currentDoorState = state;
    std::cout << "DOOR STATE SET: " << state << std::endl;
}

void System::setDoorStatesAndSave(const doorState_t& state)
{
    this->setDoorStates(state);
    this->saveSystem(EEPROM_ADDR_DOOR_STATE);
}

void System::setWaitingForCommand(bool waiting)
{
    waitingForCommand = waiting;
}

void System::setRecoverFromPowerOff(bool recoverState) 
{
    this->recoverFromPowerOff = recoverState;
}

void System::setMotor(const std::shared_ptr<Motor> &mtr)
{
    this->motor = mtr;
}

void System::setButtons(const std::shared_ptr<Switches> &btns)
{
    this->buttons = btns;
}


/* SYSTEM GETTERS */

calibrationState_t System::getCalibState() const
{
    return this->calibState;
}

bool System::getDoorErrorState() const
{
    return this->motor->getErrorState();
}

doorState_t System::getCurrentDoorState() const
{
    return currentDoorState;
}

doorState_t System::getPreviousDoorState() const
{
    return previousDoorState;
}

bool System::getWaitingForCommand() const
{
    return waitingForCommand;
}

bool System::getRecoverFromPowerOff() const 
{
    return this->recoverFromPowerOff;
}



/* * * * * *  I O    D E V I C E S  * * * * * */


/* MQTT INTERFACE */

void System::connectToWifi() const
{
    this->mqttBroker->initWiFi();
}

void System::connectToMQTTServer() const
{
    if (!this->mqttBroker->getMqttStatus())
    {
        // [1]: Attempt to connect to a Wi-Fi.
        if (this->mqttBroker->initWiFi())
        {
            /* [2]: Create a new mqtt client instance that is used through-out the use, initiate callback functions for connection,
            subscribe & publish */
            if (this->mqttBroker->establishMQTTClient())
            {
                std::cout << "[MQTT-SYSTEM-STATUS] --> Successfully attempted connection to an MQTT broker." << std::endl;
            }
            else
            {
                std::cout << "[MQTT-SYSTEM-STATUS] --> Failure in attempt to connect to an MQTT broker, check establishMQTTClient() -method" << std::endl;
            }
        }
    }
}

void System::disconnectFromMQTTServer() const
{
    if (this->mqttBroker->disconnectMQTTClient())
    {
        std::cout << "[MQTT-SYSTEM-STATUS] --> MQTT disconnected successfully from the broker." << std::endl;
        return;
    }

    std::cout << "[MQTT-SYSTEM-STATUS] --> MQTT disconnection attempt failed, were you even connected to an active broker?" << std::endl;
}

void System::subPub(const mqttSubState_t &transmissionType, const std::string &topic, const std::optional<std::string> &msg) const
{
    if (!this->mqttBroker->getMqttStatus()) return;

    std::string errorMsg;
    if (transmissionType == UNSUBSCRIBE || transmissionType == SUBSCRIBE)
    {
            this->mqttBroker->setTopic(topic);
            this->mqttBroker->subscribeToTopic(transmissionType); // tää pitäs laittaa iffin sisää tai jotenkin tarkastaa
    }

    else
    {
        if (msg.has_value())
        {
            this->mqttBroker->setTopic(topic);
            this->mqttBroker->publishToTopic(msg.value());
        }
        else
        {
            errorMsg = "[MQTT-SUBPUB] --> msg had no value, transmission type: <PUBLISH>";
        }
    }

    if (!errorMsg.empty())std::cout << errorMsg << std::endl;
}

bool System::validateDriveCommand(const std::string &rC) const
{
    const std::optional<std::string> driveCommand = this->mqttBroker->getNextCommand();
    bool validCommand = false;

    if (driveCommand.has_value())
    {
        std::string systemMsg = "INSTRUCTION FAILED.";

        if (driveCommand == "OPEN" || driveCommand == "CLOSE" || driveCommand == "REVERSE" || driveCommand == "STOP")
        {
            if (driveCommand == rC)
            {
                systemMsg = "INSTRUCTION SUCCESSFULLY EXECUTED";
                validCommand = true;
            }
        }
        subPub(PUBLISH, MQTT_TOPIC_INSTRUCTION_RESPONSE, systemMsg);

    }
    return validCommand;
}


/* EEPROM INTERFACE */
void System::saveSystem(uint16_t addr) {
    bool value = false;

    switch (addr) {
        case EEPROM_ADDR_CALIBRATION_STATE:
            value = this->eeprom->write_byte(addr, static_cast<uint8_t>(this->getCalibState()));
            break;

        case EEPROM_ADDR_DOOR_STATE:
            value = this->eeprom->write_byte(addr, static_cast<uint8_t>(this->getCurrentDoorState()));
            break;
            
        case EEPROM_ADDR_DIRECTION:
            value = this->eeprom->write_byte(addr, this->motor->getDirection());
            break;

        case EEPROM_ADDR_STEPS_PER_ENCODER:
            value = this->eeprom->write_float(addr, this->motor->getMtrStpsPerEncder());
            break;

        default:
            std::cerr << "[ERROR-SYS-SAVE] --> invalid memory address" << std::endl;
    }
    if (!value) {
        std::cerr << "[ERROR-SYS-SAVE] --> failed saving" << std::endl;
    }
}


void System::loadSystem() {
    auto calibState = this->eeprom->read_byte(EEPROM_ADDR_CALIBRATION_STATE); // CALIB_STATE

    if (!calibState.has_value()) {
        std::cout << "[SYS-LOAD] --> CALIB_STATE: previous value not found" << std::endl;
        return;
    }
    
    if (calibState.value() == CALIBRATING) {
        std::cout << "[SYS-LOAD] --> CALIB_STATE: calibration needed" << std::endl;
        return;
    }


    auto doorState = this->eeprom->read_byte(EEPROM_ADDR_DOOR_STATE); // DOOR_STATE

    if (!doorState.has_value()) {
        std::cout << "[SYS-LOAD] --> DOOR_STATE: previous value not found" << std::endl;
        return;
    }
    

    auto direction = this->eeprom->read_byte(EEPROM_ADDR_DIRECTION); // DIRECTION

    if (!direction.has_value()) {
        std::cout << "[SYS-LOAD] --> DOOR_STATE: previous value not found" << std::endl;
        return;
    }


    auto stepsPerEncoder = this->eeprom->read_float(EEPROM_ADDR_STEPS_PER_ENCODER); // STEPS_PER_ENCODER

    if (!stepsPerEncoder.has_value()) {
        std::cout << "[SYS-LOAD] --> STPS_PER_ENC: previous value not found" << std::endl;
        return;
    }
    if (stepsPerEncoder.value() <= 0) {
        std::cout << "[SYS-LOAD] --> STPS_PER_ENC: calibration needed" << std::endl;
        return;
    }

    this->setCalibState((calibrationState_t) calibState.value());
    this->setDoorStates((doorState_t) doorState.value());
    this->motor->setMtrStpsPerEncder(stepsPerEncoder.value());
    this->motor->setDirection((bool) direction.value());

    if (calibState.value() == CALIBRATED) {
        this->buttons->setInterrupt(std::vector{PIN_SW1}, true);
        this->enableStatusLeds(true);
        if (doorState.value() == IN_BETWEEN) {
            this->recoverFromPowerOff = true;
        }
    }
}


/* STATUS LEDS INTERFACE */
void System::createStatusLeds()
{
    this->statusLeds =
    std::make_shared<StatusLeds>(
        std::vector{
            PIN_LED_1,
            PIN_LED_2,
            PIN_LED_3
        }, weak_from_this());
    }
    
void System::updateStatusLeds() const
{
    statusLeds->updateCall();
}

void System::enableStatusLeds(const bool& enbld) {
    statusLeds->setEnabled(enbld);
}


/* ########## P R I V A T E ########## */
