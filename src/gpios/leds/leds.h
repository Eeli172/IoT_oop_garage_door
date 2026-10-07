#ifndef IOT_OOP_GARAGE_DOOR_LEDS_H
#define IOT_OOP_GARAGE_DOOR_LEDS_H

#include <memory>
#include <pico/types.h>
#include <vector>

#include "../gpios.h"
#include "../../types/types.h"

class System;

class StatusLeds {
public:
    StatusLeds(const std::vector<int> &ledGpios, const std::weak_ptr<System> &sys);
    StatusLeds(const StatusLeds &) = delete;
    ~StatusLeds() = default;

    /*
    Update led states according to:
        1. System.currentDoorState
        2. System.doorErrorState
    Needs to be called constantly
    */ 
    void updateCall();

    /*
    Give the updateCall permission to turn the leds on
    */
    void setEnabled(const bool& enbld);
    
private:
    /*
    Turn all the leds off
    */
    void turnOff();
    
    /*
    Toggle this->ledValue with a timeout
    */
    void toggleValue();

    /*
    Fetch door and error states from System class
    */
    void getStates();

    /*
    Turn leds off EXCEPT for the led corresponding to this->doorState, 
        which is turned into this->ledValue -state 
    */ 
    void updateLeds();

    // OTHER CLASSES
    std::vector<std::unique_ptr<CommonGPIO>> ledGpios;
    std::weak_ptr<System> system;

    // ATTRIBUTES
    doorState_t doorState;
    bool errorState;
    bool ledValue;
    bool enabled;
    absolute_time_t time_previous;
};




#endif //IOT_OOP_GARAGE_DOOR_LEDS_H
