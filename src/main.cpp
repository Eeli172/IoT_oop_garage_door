/* HEADERS */
// STL
#include <iostream>

// CUSTOM
#include "gpios/gpios.h"
#include "interrupts/interrupts.h"
#include "motor_run/motor_run.h"
#include "motor_run/calibration/calibration.h"
#include "mqtt/mqtt.h"
#include "pico/types.h"


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
    constexpr systemBank systemBank {
        .currentState = PRE_CALIB,
        .isCalibrated = false,
        .encoderPos = 0
    };

    while (true)
    {
        switch (systemBank.currentState)
        {
            case PRE_CALIB:
                while () // SW0 ja SW2 pohjassa --> siirtyy CALIB

            break;

            case CALIB:
            break;

            case PRE_RUN:
                while () // SW1 kontrollinappi --> eri toimintoja
            break;

            case RUN:
            break;
        }
    }

    return 0;
}