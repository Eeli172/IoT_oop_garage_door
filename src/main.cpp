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
    sys->connectToMQTTServer();
    sys->loadSystem();

    // EEPROM TESTAUSTA VARTEN
    //sys->printStates();

    bool queueDummy;
    bool stoppedBySw1;

    while (true)
    {
        switch (sys->getCalibState())
        {
        case NOT_CALIBRATED:

            userButtons->setInterrupt(std::vector{PIN_SW1}, false);
            if (gpio_get(PIN_SW0) && gpio_get(PIN_SW2))
            {
                sys->setCalibStateAndSave(CALIBRATING);
            }
            break;


        case CALIBRATING:

            std::cout << "Calibration: started" << std::endl;
            stepperMotor->calibrateMotor(*encoder);
            sys->saveSystem(EEPROM_ADDR_STEPS_PER_ENCODER);
            sys->setDoorStatesAndSave(CLOSED); 
            std::cout << "Calibration: finished" << std::endl;
            
            userButtons->setInterrupt(std::vector{PIN_SW1}, true);
            sys->subPub(SUBSCRIBE, MQTT_TOPIC_RUN_INSTRUCTIONS, std::nullopt);
            sys->subPub(PUBLISH, MQTT_TOPIC_SYSTEM_STATE, "CALIBRATED");
            sys->setCalibStateAndSave(CALIBRATED);
            break;


        case CALIBRATED:

            sys->updateStatusLeds();
            switch (sys->getCurrentDoorState())
            {
            case CLOSED:

                if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->validateDriveCommand("OPEN"))
                {
                    sys->setDoorStatesAndSave(IN_BETWEEN); // PREVIOUS-STATE = CLOSED, CURRENT_STATE = IN_BETWEEN
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN");
                }
                break;

            case IN_BETWEEN:

                // Door is stopped because of STUCK or SW1.
                if (stoppedBySw1 || stepperMotor->getErrorState())
                {
                    if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->validateDriveCommand("REVERSE"))
                    {
                        if (stepperMotor->getErrorState())
                        {
                            sys->subPub(PUBLISH,MQTT_TOPIC_ERROR_STATE, "STUCK WAS DETECTED");
                            stepperMotor->clearErrorState(); // tätä ei tarvii, errorState nollataan calibrateMotorissa ja stepit nollataan runMotorissa
                            /* sys->setCalibStateAndSave(NOT_CALIBRATED);
                             * sys->setDoorStatesAndSave(CLOSED);
                             * sys->subPub(UNSUBSCRIBE, MQTT_TOPIC_RUN_INSTRUCTIONS, std::nullopt);
                             */
                        }
                    }
                    else break;
                }

                stoppedBySw1 = stepperMotor->runMotor(*encoder);

                // Motor reached a limit switch { OPEN, CLOSED }
                if (!stoppedBySw1 && !stepperMotor->getErrorState())
                {
                    if (sys->getPreviousDoorState() == CLOSED &&
                   !stepperMotor->getDirection())
                    {
                        sys->setDoorStatesAndSave(OPEN);
                        sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "OPEN");
                    }
                    else if (sys->getPreviousDoorState() == OPEN &&
                        stepperMotor->getDirection())
                    {
                        sys->setDoorStatesAndSave(CLOSED);
                        sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "CLOSED");
                    }
                    else
                    {
                        // Door was reversed and reached the same limit.
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
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN");
                }
                break;
            }
        }

        /*
        // EEPROM TESTAUSTA VARTEN
        if (gpio_get(PIN_SW2)) {
            sys->printStates();
            while (gpio_get(PIN_SW2));
            sleep_ms(200);
            
            sys->loadSystem();
            sys->printStates();
        }
        */
    }
}
