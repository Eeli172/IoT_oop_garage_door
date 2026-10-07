#include <vector>
#include <memory>
#include <hardware/gpio.h>
#include <pico/time.h>

#include "leds.h"
#include "../../macros/macros.h"
#include "system/system.h"


/* ########## P U B L I C ########## */

StatusLeds::StatusLeds(const std::vector<int> &ledGpios, const std::weak_ptr<System> &sys) :
    system(sys),
    doorState(CLOSED), 
    errorState(false),
    ledValue(false), 
    enabled(false),
    time_previous(0)
{
    for (const auto &ledGpio: ledGpios)
    {
        this->ledGpios.emplace_back(std::make_unique<CommonGPIO>(ledGpio, false, false, false));
    }
}


/*
Update led states according to:
    1. System.currentDoorState
    2. System.doorErrorState
Needs to be called constantly
*/ 
void StatusLeds::updateCall() {
    if (this->enabled) {        
        this->getStates();
        if (errorState) this->toggleValue();
        else ledValue = true;
        
        this->updateLeds();
    }
    else {
        turnOff();
    }
}

/*
Give the updateCall permission to turn the leds on
*/
void StatusLeds::setEnabled(const bool& enbld) {
    this->enabled = enbld;
    updateCall();
}


/* ########## P R I V A T E ########## */


/*
Turn all the leds off
*/
void StatusLeds::turnOff() {
    gpio_put(PIN_LED_1, false);
    gpio_put(PIN_LED_2, false);
    gpio_put(PIN_LED_3, false);
}

/*
Toggle this->ledValue with a timeout
*/
void StatusLeds::toggleValue() {
    if (absolute_time_diff_us(time_previous, get_absolute_time()) > LED_BLINK_TIMEOUT_MS * 1000) {
        ledValue = !ledValue;
        time_previous = get_absolute_time();
    }    
}    

/*
Fetch door and error states from System class
*/
void StatusLeds::getStates() {
    if (auto systemPtr = this->system.lock()) {
        this->doorState = systemPtr->getCurrentDoorState();
        this->errorState = systemPtr->getDoorErrorState();
    }    
}    

/*
Turn leds off EXCEPT for the led corresponding to this->doorState, 
    which is turned into this->ledValue -state 
*/ 
void StatusLeds::updateLeds() {

    switch (doorState) {
    case OPEN:    
        gpio_put(PIN_LED_1, ledValue);
        gpio_put(PIN_LED_2, false);
        gpio_put(PIN_LED_3, false);
        break;

    case IN_BETWEEN:    
        gpio_put(PIN_LED_1, false);
        gpio_put(PIN_LED_2, ledValue);
        gpio_put(PIN_LED_3, false);
        break;

    case CLOSED:    
        gpio_put(PIN_LED_1, false);
        gpio_put(PIN_LED_2, false);
        gpio_put(PIN_LED_3, ledValue);
        break;
    }    
}    

