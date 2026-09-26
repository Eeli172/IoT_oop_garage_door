/* HEADERS */
// STL
#include <hardware/gpio.h>
#include <iostream>
#include <ostream>
#include <sys/stat.h>
#include <memory>
#include <vector>

// CUSTOM
#include "system/sys.h"
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


    System sys;
    bool queueDummy;
    bool stoppedBySw1;

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

    stepperMotor->createStatusLeds();

    while (true)
    {
        switch (sys.getSysState())
        {
        case PRE_CALIB:
            if (gpio_get(PIN_SW0) && gpio_get(PIN_SW2))
            {
                sys.setSysState(CALIBRATING);
            }
            break;


        case CALIBRATING:
            // [1] Calling class function calibrateMotor
            std::cout << "Calibration: started" << std::endl;
            limitSwitches->enableInterrupts(std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC});
            stepperMotor->calibrateMotor(*encoder);
            std::cout << "Calibration: finished" << std::endl;
            userButtons->enableInterrupts(std::vector{PIN_SW1});
            sys.setSysState(CALIBRATED);
            break;


        case CALIBRATED:

            stepperMotor->updateStatusLeds();

            switch (stepperMotor->getCurrentState())
            {
            case CLOSED:
                // press sw1 to open
                if (queue_try_remove(&sw1EventQ, &queueDummy))
                {
                    stepperMotor->setCurrentState(IN_BETWEEN); // PREVIOUS-STATE = CLOSED, CURRENT_STATE = IN_BETWEEN
                }
                break;

            case IN_BETWEEN:

                if (stepperMotor->isDoorStuck())
                {
                    if (queue_try_remove(&sw1EventQ, &queueDummy))
                    {
                        stepperMotor->clearDoorStuck();

                        // Leave an active limit, otherwise reverse normally.
                        if (!stepperMotor->moveAwayFromLimit())
                        {
                            stepperMotor->reverseDirection();
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                stoppedBySw1 = stepperMotor->runMotor(*encoder);


                if (stepperMotor->isDoorStuck())
                {
                    break;
                }

                if (stoppedBySw1)
                {
                    while (!queue_try_remove(&sw1EventQ, &queueDummy))
                    {
                    }
                }
                else
                {
                    if (stepperMotor->getPreviousState() == CLOSED &&
                        !stepperMotor->getDirection())
                    {
                        stepperMotor->setCurrentState(OPEN);
                    }
                    else if (stepperMotor->getPreviousState() == OPEN &&
                        stepperMotor->getDirection())
                    {
                        stepperMotor->setCurrentState(CLOSED);
                    }
                    else
                    {
                        stepperMotor->setCurrentState(
                            stepperMotor->getPreviousState());
                    }
                }

                break;

            case OPEN:
                // press sw1 to close
                if (queue_try_remove(&sw1EventQ, &queueDummy))
                {
                    stepperMotor->setCurrentState(IN_BETWEEN); // PREVIOUS-STATE = OPEN, CURRENT_STATE = IN_BETWEEN
                }
                break;
            }
        }
    }
}
