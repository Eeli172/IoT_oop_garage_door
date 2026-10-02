#include <vector>
#include <memory>
#include <hardware/gpio.h>
#include <pico/time.h>

#include "leds.h"
#include "../../macros/macros.h"
#include "system/system.h"



// SYSTEM STATUS LEDS
StatusLeds::StatusLeds(const std::vector<int> &ledGpios, const std::weak_ptr<System> &sys) :
    system(sys),
    doorState(CLOSED), 
    errorState(false),
    ledValue(false), 
    time_previous(0)
{
    for (const auto &ledGpio: ledGpios)
    {
        this->ledGpios.emplace_back(std::make_unique<CommonGPIO>(ledGpio, false, false, false));
    }
}
void StatusLeds::updateCall() {
    this->getStates();
    
    if (errorState) this->toggleValue();
    else ledValue = true;

    this->updateLeds();
}
void StatusLeds::getStates() {
    if (const auto &systemPtr = this->system.lock()) { // TÄSSÄ VIRHE, ei saada palautettua .lock() shared_ptria systemistä
        this->doorState = systemPtr->getCurrentDoorState();
        this->errorState = systemPtr->getDoorErrorState();
        printf("DOOR_STATE: %d\n", this->doorState);
        printf("ERROR_STATE: %d\n", this->errorState);

    }
}
void StatusLeds::toggleValue() {
    if (absolute_time_diff_us(time_previous, get_absolute_time()) > LED_BLINK_TIMEOUT_MS * 1000) {
        ledValue = !ledValue;
        time_previous = get_absolute_time();
    }
}
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