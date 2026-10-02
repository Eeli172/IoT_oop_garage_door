#ifndef IOT_OOP_GARAGE_DOOR_LEDS_H
#define IOT_OOP_GARAGE_DOOR_LEDS_H

#include <memory>
#include <pico/types.h>
#include <vector>

#include "../gpios.h"
#include "../../types/types.h"

class System;

// LED HUB (GPIO: 20, 21, 22)
class StatusLeds {
public:
    StatusLeds(const std::vector<int> &ledGpios, const std::weak_ptr<System> &sys);
    StatusLeds(const StatusLeds &) = delete;
    ~StatusLeds() = default;

    void updateCall();
    void getStates();
    void updateLeds();
    void toggleValue();


private:
    std::vector<std::unique_ptr<CommonGPIO>> ledGpios;
    std::weak_ptr<System> system;

    doorState_t doorState;
    bool errorState;
    bool ledValue;
    absolute_time_t time_previous;
};




#endif //IOT_OOP_GARAGE_DOOR_LEDS_H
