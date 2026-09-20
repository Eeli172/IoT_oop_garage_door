/* HEADERS */
// STL
#include <iostream>
#include <initializer_list>
#include "pico/types.h"
#include "array"

// CUSTOM

#include "gpios/gpios.h"
#include "interrupts/interrupts.h"
#include "motor_run/motor_run.h"
#include "mqtt/mqtt.h"
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
    bool queueInput;
    std::unique_ptr<Motor> stepperMotor;

} systemBank;

int main() {
    stdio_init_all();

    systemBank sB {
        .currentState = PRE_CALIB,
        .isCalibrated = false,
        .encoderPos = 0,
        .queueInput = false,
        .stepperMotor = nullptr
    };

    initQueues();
    Switches SwitchManager(SW0, SW1, SW2, true, true, true);
    SwitchManager.firstInitInterrupts();
    SwitchManager.setInterrupt(SW1, false);

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
                Motor stepperMotor ({COIL_1, COIL_2, COIL_3, COIL_4}, false);
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