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


    /*
    sys.addMotor(stepperMotor);
    sys.loadSystem();
    */


    initQueues();
    encoder->enableInterrupts();
    sys->createStatusLeds();
    sys->setMotor(stepperMotor);
    sys->connectToMQTTServer();


    bool queueDummy;
    bool stoppedBySw1;

    while (true)
    {
        sleep_ms(1);

        switch (sys->getCalibState())
        {
        case PRE_CALIB:
            if (gpio_get(PIN_SW0) && gpio_get(PIN_SW2))
            {
                sys->setCalibState(CALIBRATING);
            }
            break;


        case CALIBRATING:
            // [1] Calling class function calibrateMotor
            std::cout << "Calibration: started" << std::endl;
            limitSwitches->enableInterrupts(std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC});
            stepperMotor->calibrateMotor(*encoder);
            std::cout << "Calibration: finished" << std::endl;
            userButtons->enableInterrupts(std::vector{PIN_SW1});
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
                    sys->setCurrentDoorState(IN_BETWEEN); // PREVIOUS-STATE = CLOSED, CURRENT_STATE = IN_BETWEEN
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
                    sys->setCurrentDoorState(OPEN);
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "OPEN");
                }
                else if (sys->getPreviousDoorState() == OPEN &&
                    stepperMotor->getDirection())
                {
                    sys->setCurrentDoorState(CLOSED);
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "CLOSED");
                }
                else
                {
                    // Door was reversed and reached the same limit.
                    sys->setCurrentDoorState(sys->getPreviousDoorState());
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE,
                                stepperMotor->getDirection() ? "CLOSED" : "OPEN");
                }

                break;


            case OPEN:
                // press sw1 to close
                if (queue_try_remove(&sw1EventQ, &queueDummy) || sys->getDriveCommand() == "CLOSE")
                {
                    sys->setCurrentDoorState(IN_BETWEEN); // PREVIOUS-STATE = OPEN, CURRENT_STATE = IN_BETWEEN
                    sys->subPub(PUBLISH, MQTT_TOPIC_DOOR_STATE, "IN_BETWEEN");
                }
                break;
            }
        }
    }
}
