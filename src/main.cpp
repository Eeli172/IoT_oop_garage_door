/* HEADERS */
// STL
#include <iostream>
#include <ostream>
#include <any>
#include <typeinfo>
#include <sys/stat.h>
// CUSTOM
#include "sys/sys.h"
#include "pico/types.h"
#include "gpios/gpios.h"
#include "interrupts/interrupts.h"
#include "motor_run/motor_run.h"
#include "macros/macros.h"
#include "pico/stdio.h"
#include "gpios/encoder/encoder.h"

/* SYSTEM */
// SYSTEM VARIABLES STRUCT
typedef struct System
{
    stateMachine_t currentState;
    bool isCalibrated;
    uint encoderPos;
} systemBank;


int main()
{
    stdio_init_all();
    sleep_ms(500);

    const auto sB = std::make_unique<systemBank>();
    sB->currentState = PRE_CALIB;
    sB->isCalibrated = false;
    sB->encoderPos = 0;

    const auto stepperMotor = std::make_unique<Motor>(std::vector{COIL_1, COIL_2, COIL_3, COIL_4}, false);
    const auto userButtons = std::make_unique<Switches>(std::vector{SW0, SW1, SW2}, true, true, true);
    const auto limitSwitches = std::make_unique<Switches>(std::vector{LMSW_1, LMSW_2}, true, true, true);
    const auto encoder = std::make_unique<Encoder>(ENCODER_A, ENCODER_B, false);

    initQueues();
    userButtons->firstInitInterrupts({true, false, true});
    encoder->initInterrupt();

    bool queueInput;
    bool sw1Pressed;

    std::cout << "Boot1" << std::endl;

    while (true)
    {

        //Encoder in main
        while (queue_try_remove(&encAEventQ, &queueInput))
        {
            if (const EncoderState dir = encoder->checkDirection(); dir == EncoderState::Clockwise)
            {
                std::cout << "Encoder: Clockwise" << std::endl;
            }
            else if (dir == EncoderState::CounterClockwise)
            {
                std::cout << "Encoder: Counter-clockwise" << std::endl;
            }
        }

        switch (sB->currentState)
        {
        case PRE_CALIB:
            while (queue_try_remove(&sw0EventQ, &queueInput) && queue_try_remove(&sw2EventQ, &queueInput))
            {
                sB->currentState = CALIBRATING;
            }
            break;

        case CALIBRATING:
            // [1] Calling class function calibrateMotor
            limitSwitches->firstInitInterrupts({true, true});
            std::cout << "Calibration start: " << std::endl;
            stepperMotor->calibrateMotor();
            std::cout << "Calibration finished" << std::endl;
            userButtons->modifyInterrupts(std::vector{SW0, SW1, SW2}, std::vector{false, true, false});
            sB->currentState = CALIBRATED;
            break;

        case CALIBRATED:

            while (queue_try_remove(&sw1EventQ, &sw1Pressed))

            switch (stepperMotor->getState())
            {
            case CLOSED:
                if (sw1Pressed) {
                    printf("CLOSED\n");
                    stepperMotor->setCurrentState(OPENING);
                    sw1Pressed = false;
                }
                break;

            case OPENING:
                stepperMotor->runMotor(false);
                stepperMotor->setCurrentState(OPEN);
                break;

            case OPEN:
                if (sw1Pressed) {
                    printf("OPEN\n");
                    stepperMotor->setCurrentState(CLOSING);
                    sw1Pressed = false;
                }
                break;
    
            case CLOSING:
                stepperMotor->runMotor(true);
                stepperMotor->setCurrentState(CLOSED);
                break;
            }
        }
    }
}