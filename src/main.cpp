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
    const auto userButtons = std::make_unique<Switches>(
        std::vector{PIN_SW0, PIN_SW1, PIN_SW2}, true, true, true
    );
    const auto limitSwitches = std::make_unique<Switches>(
        std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC}, true, true, true
    );
    const auto encoder = std::make_unique<Encoder>(
        PIN_ENCODER_A, PIN_ENCODER_B, false
    );



    initQueues();

    encoder->enableInterrupts();
    userButtons->setInterrupt(std::vector{PIN_SW1}, true);
    limitSwitches->setInterrupt(std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC}, true);

    sys->setMotor(stepperMotor);
    sys->createStatusLeds();
    sys->connectToMQTTServer();

    /*
    sys->loadSystem();
    sleep_ms(5);
    sys->setCalibState(NOT_CALIBRATED);
    std::cout << "calib: " << sys->getCalibState() << std::endl;
    sleep_ms(5);
    std::cout << "door: " << sys->getCurrentDoorState() << std::endl;
    sleep_ms(5);
    //stepperMotor->setMtrStpsPerEncder(0);
    std::cout << "steps: " << stepperMotor->getMtrStpsPerEncder() << std::endl;
    sleep_ms(5);
    */


    bool queueDummy;
    bool stoppedBySw1;

    while (true)
    {
        sleep_ms(1);

        switch (sys->getCalibState())
        {
        case NOT_CALIBRATED:
            if (gpio_get(PIN_SW0) && gpio_get(PIN_SW2))
            {
                sys->setCalibState(CALIBRATING);
            }
            break;


        case CALIBRATING:
            userButtons->setInterrupt(std::vector{PIN_SW1}, false);
            limitSwitches->setInterrupt(std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC}, true);
            
            std::cout << "Calibration: started" << std::endl;
            stepperMotor->calibrateMotor(*encoder);
            std::cout << "Calibration: finished" << std::endl;

            //sys->saveStepsPerEncoder();

            userButtons->setInterrupt(std::vector{PIN_SW1}, true);
            sys->setCalibState(CALIBRATED);
            sys->subPub(PUBLISH, MQTT_TOPIC_SYSTEM_STATE, "CALIBRATED");
            break;


        case CALIBRATED:

            sys->updateStatusLeds();
            switch (sys->getCurrentDoorState())
            {
            case CLOSED:
                // press sw1 to open
                if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->getDriveCommand() == "OPEN")
                {
                    sys->setDoorStates(IN_BETWEEN); // PREVIOUS-STATE = CLOSED, CURRENT_STATE = IN_BETWEEN
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN");
                }
                break;

            case IN_BETWEEN:

                // Door is stopped because of STUCK or SW1.
                if (sys->getWaitingForCommand())
                {
                    if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->getDriveCommand() == "REVERSE")
                    {
                        sys->setWaitingForCommand(false);
                        stepperMotor->clearErrorState();
                    }
                    else
                    {
                        break;
                    }
                }

                stoppedBySw1 = stepperMotor->runMotor(*encoder);

                // Motor detected a stuck door.
                if (stepperMotor->getErrorState())
                {
                    //  printf("STUCK in between\n");
                    sys->subPub(
                        PUBLISH,
                        MQTT_TOPIC_ERROR_STATE,
                        "STUCK"
                    );

                    sys->setWaitingForCommand(true);
                    // sys->setCalibState(NOT_CALIBRATED); 
                    break;
                }

                // Motor was stopped by SW1.
                if (stoppedBySw1)
                {
                    //  printf("stopped by SW1 \n");
                    sys->setWaitingForCommand(true);
                    break;
                }

                // Motor reached a limit switch.
                if (sys->getPreviousDoorState() == CLOSED &&
                    !stepperMotor->getDirection())
                {
                    sys->setDoorStates(OPEN);
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "OPEN");
                }
                else if (sys->getPreviousDoorState() == OPEN &&
                    stepperMotor->getDirection())
                {
                    sys->setDoorStates(CLOSED);
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "CLOSED");
                }
                else
                {
                    // Door was reversed and reached the same limit.
                    sys->setDoorStates(sys->getPreviousDoorState());
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE,
                                stepperMotor->getDirection() ? "CLOSED" : "OPEN");
                }

                break;


            case OPEN:
                // press sw1 to close
                if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->getDriveCommand() == "CLOSE")
                {
                    sys->setDoorStates(IN_BETWEEN); // PREVIOUS-STATE = OPEN, CURRENT_STATE = IN_BETWEEN
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN");
                }
                break;
            }
        }
    }
}
