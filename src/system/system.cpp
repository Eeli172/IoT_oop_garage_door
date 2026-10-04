/* STL */
#include <iostream>
#include <memory>
#include <ostream>

/* CUSTOM */
#include "system.h"
#include "eeprom/eeprom.h"
#include "gpios/leds/leds.h"

#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "types/types.h"


System::System() :
calibState(PRE_CALIB),
currentDoorState(CLOSED),
previousDoorState(CLOSED),
mqttBroker(std::make_unique<MQTTManager>()),
eeprom(std::make_unique<Eeprom>())
{}


// SETTER FUNCTIONS
void System::setCalibState(const calibrationState_t& state)
{
    this->calibState = state;
    // this->saveSystem(EEPROM_ADDR_CALIBRATION_STATE);
}

void System::setCurrentDoorState(const doorState_t& state)
{
    previousDoorState = currentDoorState;
    currentDoorState = state;
}

void System::setMotor(const std::shared_ptr<Motor> &Mtr)
{
    this->motor = Mtr;
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

std::string System::getDriveCommand() const
{
    return this->mqttBroker->getNextCommand();
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



/* EEPROM INTERFACE */
void System::saveSystem(uint16_t addr) {
    int temp{0};

    switch (addr) {
        case EEPROM_ADDR_CALIBRATION_STATE:
            temp = static_cast<int>(this->getCalibState());
            break;

        case EEPROM_ADDR_DOOR_STATE:
            temp = static_cast<int>(getCurrentDoorState());
            break;

        default:
            std::cerr << "[ERROR-SYSTEM-SAVE] --> invalid memory address" << std::endl;
    }

    if (this->eeprom->write(addr, temp)) {
        std::cerr << "[ERROR-SYSTEM-SAVE] --> Failed saving" << std::endl;
    }
}

void System::loadSystem() {
    int temp{0};

    if (this->eeprom->read(EEPROM_ADDR_CALIBRATION_STATE, &temp)) {
        std::cout << "[SYS-LOAD-CALIB_STATE] --> previous value not found" << std::endl;
        return;
    }
    std::cout << "[SYS-LOAD-CALIB_STATE] --> previous value found" << std::endl;
    this->setCalibState((calibrationState_t) temp);

    if (this->eeprom->read(EEPROM_ADDR_DOOR_STATE, &temp)) {
        std::cout << "[SYS-LOAD-DOOR_STATE] --> previous value not found" << std::endl;
        return;
    }
    std::cout << "[SYS-LOAD-DOOR_STATE] --> previous value found" << std::endl;
    setCurrentDoorState((doorState_t) temp);
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