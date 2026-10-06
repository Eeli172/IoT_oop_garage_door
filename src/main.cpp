/* HEADERS */
// STL
#include <hardware/gpio.h>
#include <iostream>
#include <ostream>
#include <memory>
#include <vector>

// CUSTOM
#include "system/system.h"
#include "gpios/gpios.h"
#include "interrupts/interrupts.h"
#include "motor_run/motor_run.h"
#include "macros/macros.h"
#include "pico/stdio.h"
#include "gpios/encoder/encoder.h"

/* SYSTEM */


int main()
{
    stdio_init_all();
    sleep_ms(500);
    std::cout << "\n BOOT\n" << std::endl;

    const auto sys = std::make_shared<System>();
    const auto stepperMotor = std::make_shared<Motor>(
        std::vector{PIN_COIL_1, PIN_COIL_2, PIN_COIL_3, PIN_COIL_4}, false
    );
    const auto userButtons = std::make_shared<Switches>(
        std::vector{PIN_SW0, PIN_SW1, PIN_SW2}, true, true, true
    );
    const auto encoder = std::make_unique<Encoder>(
        PIN_ENCODER_A, PIN_ENCODER_B, false
    );

    initQueues();

    encoder->enableInterrupts();

    sys->setMotor(stepperMotor);
    sys->setButtons(userButtons);
    sys->createStatusLeds();
    sys->loadSystem();
    sys->connectToWifi();

    bool queueDummy;
    bool manualStop = false;
    absolute_time_t wifiTimeout = 0;

    while (true)
    {
        sys->updateStatusLeds();

        if (absolute_time_diff_us(wifiTimeout, get_absolute_time()) >= 1000000 * CHECK_WIFI_STATUS_INTERVAL_SECONDS
            && sys->getCalibState() == CALIBRATED)
        {
            wifiTimeout = get_absolute_time();
            sys->connectToMQTTServer();
        }

        switch (sys->getCalibState())
        {
        case NOT_CALIBRATED:
            if ((gpio_get(PIN_SW0) && gpio_get(PIN_SW2)))
            {
                sys->setCalibStateAndSave(CALIBRATING);
            }
            break;


        case CALIBRATING:

            std::cout << "Calibration: started" << std::endl;
            sys->enableStatusLeds(false);
            stepperMotor->calibrateMotor(*encoder);
            sys->saveSystem(EEPROM_ADDR_STEPS_PER_ENCODER);
            sys->setDoorStatesAndSave(CLOSED); 
            std::cout << "Calibration: finished" << std::endl;
            
            sys->enableStatusLeds(true);
            userButtons->setInterrupt(std::vector{PIN_SW1}, true);
            sys->setCalibStateAndSave(CALIBRATED);
            sys->connectToMQTTServer();
            sys->subPub(PUBLISH, MQTT_TOPIC_SYSTEM_STATE, "CALIBRATED");
            wifiTimeout = get_absolute_time();
            break;


        case CALIBRATED:

            switch (sys->getCurrentDoorState())
            {
            case CLOSED:

                if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->validateDriveCommand("OPEN"))
                {
                    sys->setDoorStatesAndSave(IN_BETWEEN); // PREVIOUS-STATE = CLOSED, CURRENT_STATE = IN_BETWEEN

                    sys->saveSystem(EEPROM_ADDR_DIRECTION); // Saves the new direction to EEPROM (IN_BETWEEN)

                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN"); // Publishes the state change via MQTT
                }
                break;
            
                
            case IN_BETWEEN:

                // after reboot this stops and akss for button before continuing
                // without that it will automatically go to limitswitch
                if (sys->getWaitingForCommand())
                {
                    manualStop = true;
                    sys->setWaitingForCommand(false);
                }

                if (manualStop) // STOPPED BY SW1 (or mqtt command?)
                {
                    if (!queue_try_remove(&sw1EventQ, &queueDummy) && !sys->validateDriveCommand("REVERSE")) {
                        break;
                    }
                }

                manualStop = stepperMotor->runMotor(*encoder, [&sys](const std::string& cmd)
                {
                    return sys->validateDriveCommand(cmd);
                });

                if (stepperMotor->getErrorState()) // ERROR --> CALIBRATION NEEDED
                {
                    userButtons->setInterrupt(std::vector{PIN_SW1}, false);
                    sys->subPub(PUBLISH, MQTT_TOPIC_ERROR_STATE, "STUCK WAS DETECTED");
                    sys->disconnectFromMQTTServer();
                    sys->setCalibStateAndSave(NOT_CALIBRATED);
                    manualStop = false;
                    break;
                }

                if (!manualStop) // Motor reached a limit switch { OPEN, CLOSED }
                {
                    if (sys->getPreviousDoorState() == CLOSED && !stepperMotor->getDirection())
                    {
                        sys->setDoorStatesAndSave(OPEN);
                        sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "OPEN");
                    }
                    else if (sys->getPreviousDoorState() == OPEN && stepperMotor->getDirection())
                    {
                        sys->setDoorStatesAndSave(CLOSED);
                        sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "CLOSED");
                    }
                    else // Door was reversed and reached the same limit.
                    {
                        sys->setDoorStatesAndSave(sys->getPreviousDoorState());
                        sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE,
                                    stepperMotor->getDirection() ? "CLOSED" : "OPEN");
                    }
                }
                break;


            case OPEN:

                if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->validateDriveCommand("CLOSE"))
                {
                    sys->setDoorStatesAndSave(IN_BETWEEN); // PREVIOUS-STATE = OPEN, CURRENT_STATE = IN_BETWEEN

                    sys->saveSystem(EEPROM_ADDR_DIRECTION); // Saves the new direction to EEPROM (IN_BETWEEN)

                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN"); // Publishes the state change via MQTT
                }
                break;
            }
        }
    }
}
