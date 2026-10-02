#include <vector>
#include <memory>
#include <hardware/gpio.h>
#include <pico/time.h>

#include "leds.h"
#include "../../macros/macros.h"
#include "../../motor_run/motor_run.h"



// SYSTEM STATUS LEDS
StatusLeds::StatusLeds(const std::vector<int> &ledGpios, std::weak_ptr<Motor> mtr) : 
    motor(mtr),
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
    if (auto motorPtr = this->motor.lock()) {
        this->doorState = motorPtr->getCurrentDoorState();
        this->errorState = motorPtr->getErrorState();
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