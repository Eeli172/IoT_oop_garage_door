/* HEADERS */
// STL
#include <iostream>
#include <ostream>
#include <array>

// CUSTOM

#include "pico/types.h"
#include "gpios/gpios.h"
#include "interrupts/interrupts.h"
#include "motor_run/motor_run.h"
#include "macros/macros.h"
#include "pico/stdio.h"
#include "pico/stdio/driver.h"


/* SYSTEM */
// STATE MACHINE ENUM
typedef enum { PRE_CALIB, CALIB, PRE_RUN, RUN } stateMachine;

// SYSTEM VARIABLES STRUCT
typedef struct System
{
    stateMachine currentState;
    bool isCalibrated;
    uint encoderPos;
} systemBank;

int main() {
    stdio_init_all();
    sleep_ms(2000);

    const auto sB = std::make_unique<systemBank>();
    sB->currentState = PRE_CALIB;
    sB->isCalibrated = false;
    sB->encoderPos = 0;


    const auto stepperMotor = std::make_unique<Motor>(std::vector {COIL_1, COIL_2, COIL_3, COIL_4}, false);
    const auto userButtons = std::make_unique<Switches>(std::vector {SW0, SW1, SW2}, true, true, true);
    const auto limitSwitches = std::make_unique<Switches>(std::vector {LMSW_1, LMSW_2}, true, true, true);
    
    initQueues();
    userButtons->firstInitInterrupts({true, false, true});
    
    bool queueInput;

    std::cout << "Boot1" << std::endl;

    while (true)
    {
        switch (sB->currentState)
        {
            case PRE_CALIB:
            while (queue_try_remove(&sw0EventQ, &queueInput) && queue_try_remove(&sw2EventQ, &queueInput))
            {
                sB->currentState = CALIB;
            }
            break;
            
            case CALIB:
                // [1] Calling class function calibrateMotor
                limitSwitches->firstInitInterrupts({true, true});
                std::cout << "Calibration start: " << std::endl;
                stepperMotor->calibrateMotor();
                sB->currentState = PRE_RUN;
            break;

            case PRE_RUN:
                std::cout << "Run start: " << std::endl;

                // SW1 kontrollinappi --> eri toimintoja
            break;

            case RUN:
            break;
        }
    }
}