/* HEADERS */
// STL
#include <iostream>
#include <ostream>
#include "array"

// CUSTOM

#include "pico/types.h"
#include "gpios/gpios.h"
#include "interrupts/interrupts.h"
#include "motor_run/motor_run.h"
#include "macros/macros.h"
#include "pico/stdio.h"


/* SYSTEM */
// STATE MACHINE ENUM
typedef enum { PRE_CALIB, CALIB, PRE_RUN, RUN } stateMachine;

// SYSTEM VARIABLES STRUCT
typedef struct System
{
    stateMachine currentState;
    bool isCalibrated;
    uint encoderPos;
    std::unique_ptr<Motor> stepperMotor;

} systemBank;

int main() {
    stdio_init_all();
    std::cout << "Boot1" << std::endl;
    
    System sB {
        .currentState = PRE_CALIB,
        .isCalibrated = false,
        .encoderPos = 0,
        .stepperMotor = std::make_unique<Motor>(std::array {COIL_1, COIL_2, COIL_3, COIL_4}, false)
    };
    
    initQueues();
    Switches SwitchManager({SW0, SW1, SW2}, true, true, true);
    SwitchManager.firstInitInterrupts();
    
    bool queueInput;
    
    while (true)
    {
        switch (sB.currentState)
        {
            case PRE_CALIB:
            while (queue_try_remove(&sw0EventQ, &queueInput) && queue_try_remove(&sw2EventQ, &queueInput))
            {
                std::cout << "Calibration start: " << std::endl;
                sB.currentState = CALIB;
            }
            break;
            
            case CALIB:
                // [1] Calling class function calibrateMotor
                sB.stepperMotor->calibrateMotor();
                sB.currentState = PRE_RUN;
            break;

            case PRE_RUN:
                std::cout << "Run start: " << std::endl;
                // SW1 kontrollinappi --> eri toimintoja
            break;

            case RUN:
            break;
        }
        sleep_ms(10);
    }

    return 0;
}