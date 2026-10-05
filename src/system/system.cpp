/* STL */
#include <cmath>
#include <iostream>
#include <memory>
#include <ostream>

/* CUSTOM */
#include "system.h"
#include "eeprom/eeprom.h"
#include "gpios/leds/leds.h"

#include "types/types.h"


System::System() :
calibState(NOT_CALIBRATED),
currentDoorState(CLOSED),
previousDoorState(CLOSED),
waitingForCommand(false),
mqttBroker(std::make_unique<MQTTManager>()),
eeprom(std::make_unique<Eeprom>())
{}


// SETTER FUNCTIONS
void System::setCalibState(const calibrationState_t& state)
{
    this->calibState = state;
    //this->saveSystem(EEPROM_ADDR_CALIBRATION_STATE);
    std::cout << "CALIB STATE: " << state << std::endl;
}

void System::setDoorStates(const doorState_t& state)
{
    this->previousDoorState = this->currentDoorState;
    this->currentDoorState = state;
    //this->saveSystem(EEPROM_ADDR_DOOR_STATE);
    std::cout << "DOOR STATE: " << state << std::endl;
}

void System::setMotor(const std::shared_ptr<Motor> &mtr)
{
    this->motor = mtr;
}

void System::setWaitingForCommand(bool waiting)
{
    waitingForCommand = waiting;
}


// GETTER FUNCTIONS
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


/* MQTT INTERFACE */
void System::connectToMQTTServer()
{
    // [1]: Attempt to connect to a Wi-Fi.
    if (this->mqttBroker->initWiFi())
    {
        /* [2]: Create a new mqtt client instance, initiate callback functions for connection,
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

        if (driveCommand == "OPEN" || driveCommand == "CLOSE" || driveCommand == "REVERSE")
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
void System::saveSystem(uint16_t addr, int value, uint16_t addrIncrement) {

    switch (addr) {
        case EEPROM_ADDR_CALIBRATION_STATE:
            value = static_cast<int>(this->getCalibState());
            break;

        case EEPROM_ADDR_DOOR_STATE:
            value = static_cast<int>(getCurrentDoorState());
            break;

        case EEPROM_ADDR_STEPS_PER_ENCODER:
            addr += addrIncrement;
            break;

        default:
            std::cerr << "[ERROR-SYSTEM-SAVE] --> invalid memory address" << std::endl;
    }

    if (this->eeprom->write(addr, value)) {
        std::cerr << "[ERROR-SYSTEM-SAVE] --> Failed saving" << std::endl;
    }
}


void System::saveStepsPerEncoder() {
    uint32_t floatStepsRounded = std::round(FLOAT_MULTIPLIER*motor->getMtrStpsPerEncder());

    for (uint i = 0; i < sizeof(uint32_t); i++) {
        uint8_t splitSteps = (floatStepsRounded >> (i * BYTE_BITS)) & 0xFF;
        saveSystem(EEPROM_ADDR_STEPS_PER_ENCODER, splitSteps, i*2);
    }
}



void System::loadSystem() {
    int temp{0};

    if (this->eeprom->read(EEPROM_ADDR_CALIBRATION_STATE, &temp)) 
    {
        std::cout << "[SYS-LOAD-CALIB_STATE] --> previous value not found" << std::endl;
        return;
    }
    else
    {
        std::cout << "[SYS-LOAD-CALIB_STATE] --> previous value found" << std::endl;
        this->setCalibState((calibrationState_t) temp);
    }
    

    temp = 0; 
    if (this->eeprom->read(EEPROM_ADDR_DOOR_STATE, &temp)) 
    {
        std::cout << "[SYS-LOAD-DOOR_STATE] --> previous value not found" << std::endl;
        return;
    }
    else 
    {
        std::cout << "[SYS-LOAD-DOOR_STATE] --> previous value found" << std::endl;
        if (temp != OPEN && temp != CLOSED) // calibration needed if the door isn't in OPEN or CLOSED state
        {
            this->setCalibState(NOT_CALIBRATED);
            return;
        }
        setDoorStates((doorState_t) temp);
    }
    

    uint32_t stepsPerEnc{0};
    for (uint i = 0; i < sizeof(uint32_t); i++) {
        if (this->eeprom->read(EEPROM_ADDR_STEPS_PER_ENCODER+i*2, &temp))
        {
            std::cout << "[SYS-LOAD-STPS_PER_ENC] --> previous value not found" << std::endl;
            return;
        }
        else
        {
            stepsPerEnc |= (static_cast<uint32_t>(static_cast<uint8_t>(temp)) << (i * BYTE_BITS));
        }
    }
    this->motor->setMtrStpsPerEncder((float)stepsPerEnc/FLOAT_MULTIPLIER);
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