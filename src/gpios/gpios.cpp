/* STL */
#include <memory>
#include <iostream>

/* CUSTOM */
#include "../interrupts/interrupts.h"
#include "../macros/macros.h"
#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h"
#include "gpios.h"



CommonGPIO::CommonGPIO(const int pin, const bool input = true, const bool pullup = true, const bool inverted = false) :
pin(pin), input(input), is_dormant(true)
{
    gpio_init(pin);

    if (pin < 0 || pin > 29)
    {
        return;
    }

    if ( const uint32_t mask = 1U << pin; (pins_in_use & mask) == 0)
    {
        pins_in_use |= mask;
        this->is_dormant = false;

        if (!input)
        {
            gpio_set_dir(pin, GPIO_OUT);
        }

        else
        {
            gpio_set_dir(pin, GPIO_IN);
        }

        if (input && pullup && inverted)
        {
            gpio_pull_up(pin);
            gpio_set_inover(pin, GPIO_OVERRIDE_INVERT);
        }
    }
}

uint32_t CommonGPIO::pins_in_use = 0;

CommonGPIO::~CommonGPIO()
{
    if (!this->is_dormant) {
        // Reset overrides and pull settings on release
        gpio_set_outover(this->pin, GPIO_OVERRIDE_NORMAL);
        gpio_set_inover(this->pin, GPIO_OVERRIDE_NORMAL);
        gpio_disable_pulls(this->pin);

        // Release the pin lock
        pins_in_use &= ~(1U << this->pin);
    }
}

CommonGPIO::operator bool() const
{
    return !this->is_dormant;
}

bool CommonGPIO::read() const
{
    if (this->is_dormant)
    {
        return false;
    }

    return gpio_get(this->pin);
}

void CommonGPIO::write(const bool value) const
{
    if (this->is_dormant)
    {
        return;
    }

    gpio_put(this->pin, value);
}

int CommonGPIO::getPin()
{
    if (this->is_dormant)
    {
        return -1;
    }
    return this->pin;
}

bool CommonGPIO::checkFlushQueue() {
    int dummy;

    switch (this->pin) {
        case SW0:
        if (!queue_is_empty(&sw0EventQ)) {
            while (queue_try_remove(&sw0EventQ, &dummy)) {}
            return true;
        }
        break;

        case SW1:
        if (!queue_is_empty(&sw1EventQ)) {
            while (queue_try_remove(&sw1EventQ, &dummy)) {}
            return true;
        }
        break;

        case SW2:
        if (!queue_is_empty(&sw2EventQ)) {
            while (queue_try_remove(&sw2EventQ, &dummy)) {}
            return true;
        }
        break;

        case LMSW_1:
        if (!queue_is_empty(&lmswEventQ)) {
            while (queue_try_remove(&lmswEventQ, &dummy)) {}
            return true;
        }

        case LMSW_2:
        if (!queue_is_empty(&lmswEventQ)) {
            while (queue_try_remove(&lmswEventQ, &dummy)) {}
            return true;
        }
        break;
    }

    return false;
}


// SWITCHES
Switches::Switches(const std::vector<int> &pins, bool input, bool pullup, bool inverted) {
    for (const auto &pin : pins) {
        switches.push_back(std::make_unique<CommonGPIO>(pin, input, pullup, inverted));
    }
}

// Vector first value corresponds to SW0, second to SW1 and so on...
void Switches::firstInitInterrupts(const std::vector<bool> &switchStates) const
{
    for (std::size_t i = 0; i < switchStates.size(); i++)
    {
        const auto &swPin = this->switches[i]->getPin();
        gpio_set_irq_enabled_with_callback(swPin, GPIO_IRQ_EDGE_FALL, switchStates[i], &commonIsr);
    }
}

// Allows toggling of multiple interrupts of multiple gpios at a time
void Switches::modifyInterrupts(const std::vector<int> &swPins, const std::vector<bool> &interruptStates) const
{
    if (swPins.size() != interruptStates.size())
    {
        std::cerr << "Parameter vectors not equal [swPins.size() != interruptStates.size()]" << std::endl;
        return;
    }

    if (swPins.size() > MAX_SWITCHES_CLASS)
    {
        std::cerr << "System does not support over ["<< MAX_SWITCHES_CLASS <<"] per class." << std::endl;
        return;
    }

    for (std::size_t i = 0; i < swPins.size(); i++)
    {
        for (const auto &swGpio: this->switches)
        {
            if (swGpio->getPin() == swPins[i])
            {
                gpio_set_irq_enabled(swPins[i], GPIO_IRQ_EDGE_FALL, interruptStates[i]);
            }
        }
    }
}

bool Switches::callFlushCheck(const std::vector<int> &checkablePins) const {
    for (const auto &pin : checkablePins) {
        for (const auto &swGpio: this->switches) {
            if (swGpio->getPin() == pin) {
                if (!swGpio->checkFlushQueue()) return false;
            }

            else {
                std::cerr << "[ERROR] --> No corresponding pin found in CommonGPIOs. " << std::endl;
                return false;
            }
        }
    }

    return true;
}

// SYSTEM STATUS LEDS
StatusLeds::StatusLeds(const std::vector<int> &ledGpios, bool inverted)
{
    for (const auto &ledGpio: ledGpios)
    {
        this->ledGpios.push_back(std::make_unique<CommonGPIO>(ledGpio, inverted));
    }
}

// Function to toggle led ON/OFF
void StatusLeds::writeLed (const int ledGpio) const
{
    for (const auto &pin : this->ledGpios)
    {
        if (pin->getPin() == ledGpio)
        {
            pin->write(!pin->read());
            break;
        }
    }
}



