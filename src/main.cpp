/* HEADERS */
// STL
#include <hardware/gpio.h>
#include <iostream>
#include <ostream>
#include <sys/stat.h>
#include <vector>

// CUSTOM
#include "sys/sys.h"
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
    
    System sys;
    bool queueDummy;
    bool stoppedBySw1;
    
    const auto stepperMotor = std::make_shared<Motor>(std::vector{PIN_COIL_1, PIN_COIL_2, PIN_COIL_3, PIN_COIL_4}, false);
    const auto userButtons = std::make_unique<Switches>(std::vector{PIN_SW0, PIN_SW1, PIN_SW2}, true, true, true);
    const auto limitSwitches = std::make_unique<Switches>(std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC}, true, true, true);
    const auto encoder = std::make_unique<Encoder>(PIN_ENCODER_A, PIN_ENCODER_B, false);
    
    initQueues();
    encoder->initInterrupt();
    
    stepperMotor->createStatusLeds();

    std::cout << "\n B O O T\n" << std::endl;
    while (true)
    {
        /* Encoder in main
        while (queue_try_remove(&encEventQ, &queueInput))
        {
            if (const EncoderState dir = encoder->checkDirection(); dir == EncoderState::Clockwise)
            {
                std::cout << "Encoder: Clockwise" << std::endl;
            }
            else if (dir == EncoderState::CounterClockwise)
            {
                std::cout << "Encoder: Counter-clockwise" << std::endl;
            }
            // INTERRUPTEJA EI ENÄÄN EKAN KERRAN JÄLKEEN TAPAHDU --> TEE ENCODER LUOKKAAN modifyInterurpts funktion.

        } */

        switch (sys.getSysState())
        {
            case PRE_CALIB:
            if (gpio_get(PIN_SW0) && gpio_get(PIN_SW2)) {
                sys.setSysState(CALIBRATING);
            }
            break;


        case CALIBRATING:
            // [1] Calling class function calibrateMotor
            std::cout << "Calibration: started" << std::endl;
            stepperMotor->calibrateMotor();
            std::cout << "Calibration: finished" << std::endl;
            userButtons->initInterrupts(std::vector {PIN_SW1}, std::vector {true});
            sys.setSysState(CALIBRATED);
            break;


        case CALIBRATED:
        stepperMotor->updateStatusLeds();
        switch (stepperMotor->getCurrentState())
        {
            case CLOSED: 
            // press sw1 to open
                if (queue_try_remove(&sw1EventQ, &queueDummy)) {
                    stepperMotor->setCurrentState(IN_BETWEEN); // PREVIOUS-STATE = CLOSED, CURRENT_STATE = IN_BETWEEN
                }
                break;

            case IN_BETWEEN:
                stoppedBySw1 = stepperMotor->runMotor();
            
                if (stoppedBySw1) // PYSÄYTETTY NAPISTA
                {
                    while (!queue_try_remove(&sw1EventQ, &queueDummy)) {
                        stepperMotor->updateStatusLeds();
                    }
                }

                else if (!stoppedBySw1) // AJOI PÄÄTYYN ASTI
                {
                    if (stepperMotor->getPreviousState() == CLOSED) // JOS TULEE VASEMMALTA (OLI SULJETTU)
                    {
                        stepperMotor->setCurrentState(OPEN);
                    }

                    else if (stepperMotor->getPreviousState() == OPEN) // JOS TULEE OIKEALTA (OLI AUKI)
                    {
                        stepperMotor->setCurrentState(CLOSED);
                    }
                }

                else; // PYSÄYTETTY ENCODERIN TOIMESTA / OVI JUMISSA (pitää muokata vielä logiikkaa)

                break;

            case OPEN:
                // press sw1 to close
                if (queue_try_remove(&sw1EventQ, &queueDummy)) {
                    stepperMotor->setCurrentState(IN_BETWEEN); // PREVIOUS-STATE = OPEN, CURRENT_STATE = IN_BETWEEN
                }
                break;
            }
        }
    }
}