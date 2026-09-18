/* HEADERS */
// STL
#include <iostream>
#include "pico/types.h"
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

} systemBank;

int main() {
    stdio_init_all();

    const systemBank systemBank {
        .currentState = PRE_CALIB,
        .isCalibrated = false,
        .encoderPos = 0
    };

    initQueues();
    Switches SwitchManager(SW0, SW1, SW2, true, true, true);
    SwitchManager.firstInitInterrupts();
    SwitchManager.setInterrupt(SW1, false);

    while (true)
    {
        switch (systemBank.currentState)
        {
            case PRE_CALIB:
                while (!queue_is_empty(&sw0EventQ) && !queue_is_empty(&sw2EventQ))
                {
                    std::cout << "C++ haisee paskalta" << std::endl;
                }// SW0 ja SW2 pohjassa --> siirtyy CALIB
            break;

            case CALIB:
            break;

            case PRE_RUN:
                while (true){} // SW1 kontrollinappi --> eri toimintoja
            break;

            case RUN:
            break;
        }
    }

    return 0;
}