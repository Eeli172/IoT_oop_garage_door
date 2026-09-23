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

    const auto stepperMotor = std::make_unique<Motor>(std::vector{PIN_COIL_1, PIN_COIL_2, PIN_COIL_3, PIN_COIL_4}, false);
    const auto userButtons = std::make_unique<Switches>(std::vector{PIN_SW0, PIN_SW1, PIN_SW2}, true, true, true);
    const auto limitSwitches = std::make_unique<Switches>(std::vector{PIN_LMSW_MTR, PIN_LMSW_ENC}, true, true, true);
    const auto encoder = std::make_unique<Encoder>(PIN_ENCODER_A, PIN_ENCODER_B, false);

    initQueues();
    encoder->initInterrupt();

    gpio_set_irq_enabled_with_callback(PIN_LMSW_MTR, GPIO_IRQ_EDGE_FALL, false, &commonIsr);
    gpio_set_irq_enabled_with_callback(PIN_LMSW_ENC, GPIO_IRQ_EDGE_FALL, false, &commonIsr);
    gpio_set_irq_enabled_with_callback(PIN_SW1, GPIO_IRQ_EDGE_FALL, false, &commonIsr);

    std::cout << "\n B O O T\n" << std::endl;
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
            if (gpio_get(PIN_SW0) && gpio_get(PIN_SW2)) {
                sys.setSysState(CALIBRATING);
            }
            break;

        case CALIBRATING:
            // [1] Calling class function calibrateMotor
            std::cout << "Calibration start: " << std::endl;
            stepperMotor->calibrateMotor();
            std::cout << "Calibration finished" << std::endl;
            gpio_set_irq_enabled(PIN_SW1, GPIO_IRQ_EDGE_FALL, true);
            sys.setSysState(CALIBRATED);
            break;

        case CALIBRATED:
            switch (stepperMotor->getState())
            {
            case CLOSED: 
                // press sw1 to open
                if (queue_try_remove(&sw1EventQ, &queueDummy)) {
                    stepperMotor->setCurrentState(OPENING);
                }
                break;
    
            case OPEN:
                // press sw1 to close
                if (queue_try_remove(&sw1EventQ, &queueDummy)) {
                    stepperMotor->setCurrentState(CLOSING);
                }
                break;
                
            case OPENING:
                // open until sw1 or limit switch is pressed
                printf("OPENING\n");
                if (stepperMotor->runMotor()) {
                    stepperMotor->setCurrentState(STOPPED);
                }
                else stepperMotor->setCurrentState(OPEN);
                break;

            case CLOSING:
                // close until sw1 or limit switch is pressed
                printf("CLOSING\n");
                if (stepperMotor->runMotor()) {
                    stepperMotor->setCurrentState(STOPPED);
                }
                else stepperMotor->setCurrentState(CLOSED);
                break;

            case STOPPED: 
                // press sw1 to continue into the opposite direction
                flushQueue(&lmswMtrEventQ);
                if (queue_try_remove(&sw1EventQ, &queueDummy)) {
                    if (stepperMotor->getDirection()) {
                        stepperMotor->setCurrentState(OPENING);
                    }
                    else stepperMotor->setCurrentState(CLOSING);
                }

            }
        }
    }
}