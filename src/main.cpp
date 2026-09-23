/* HEADERS */
// STL
#include <iostream>
#include <ostream>
#include <any>
#include <typeinfo>
#include <sys/stat.h>

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

    const auto stepperMotor = std::make_unique<Motor>(std::vector{COIL_1, COIL_2, COIL_3, COIL_4}, false);
    const auto userButtons = std::make_unique<Switches>(std::vector{SW0, SW1, SW2}, true, true, true);
    const auto limitSwitches = std::make_unique<Switches>(std::vector{LMSW_1, LMSW_2}, true, true, true);
    const auto encoder = std::make_unique<Encoder>(ENCODER_A, ENCODER_B, false);

    initQueues();
    encoder->initInterrupt();

    uint sw0Pressed;
    uint sw1Pressed;
    uint sw2Pressed;

    std::cout << "Boot1" << std::endl;
    userButtons->firstInitInterrupts({true, false, true});
    while (true)
    {

        /*Encoder in main
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

        }*/

        switch (sys.getSysState())
        {
            case PRE_CALIB:
            if (userButtons->callFlushCheck(std::vector {SW0, SW2})) {
                sys.setSysState(CALIBRATING);
            }
            break;

        case CALIBRATING:
            // [1] Calling class function calibrateMotor
            limitSwitches->firstInitInterrupts({true, true});
            std::cout << "Calibration start: " << std::endl;
            stepperMotor->calibrateMotor();
            std::cout << "Calibration finished" << std::endl;
            userButtons->modifyInterrupts(std::vector{SW1}, std::vector{true});
            sys.setSysState(CALIBRATED);
            break;

        case CALIBRATED:
            switch (stepperMotor->getState())
            {
            case CLOSED:
                if (userButtons->callFlushCheck(std::vector {SW1})) {
                    printf("CLOSED\n");
                    stepperMotor->setCurrentState(OPENING);
                    sw1Pressed = false;
                }
                break;

            case OPENING:
                userButtons->modifyInterrupts(std::vector{SW1}, std::vector{true});
                stepperMotor->runMotor(false);
                stepperMotor->setCurrentState(OPEN);
                break;

            case OPEN:
                if (userButtons->callFlushCheck(std::vector {SW1})) {
                    printf("OPEN\n");
                    stepperMotor->setCurrentState(CLOSING);
                    sw1Pressed = false;
                }
                break;
    
            case CLOSING:
                userButtons->modifyInterrupts(std::vector{SW1}, std::vector{true});
                stepperMotor->runMotor(true);
                stepperMotor->setCurrentState(CLOSED);
                break;
            }
        }
    }
}