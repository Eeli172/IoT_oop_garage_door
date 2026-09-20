#include <iostream>
#include "pico/stdlib.h"
#include "gpios.h"
#include <memory>

#include "../interrupts/interrupts.h"
#include "../macros/macros.h"

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
§
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



// SWITCH HUB
Switches::Switches(int sw0Pin, int sw1Pin, int sw2Pin, bool input, bool pullup, bool inverted) :
switches
{
    CommonGPIO(sw0Pin, input, pullup, inverted),
    CommonGPIO(sw1Pin, input, pullup, inverted),
    CommonGPIO(sw2Pin, input, pullup, inverted),
}
{}

void Switches::firstInitInterrupts()
{
    for (auto &pin : this->switches)
    {
        const int pinNum = pin.getPin();
        gpio_set_irq_enabled_with_callback(pinNum, GPIO_IRQ_EDGE_FALL, true, &commonIsr);
    }
}

void Switches::setInterrupt(const int gpio, const bool interruptStatus)
{
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, interruptStatus);
}



// LIMIT SWITCH HUB
LimitSwitches::LimitSwitches(const int lmswPin1, const int lmswPin2, const bool inverted) :
limitSwitches
{
    CommonGPIO(lmswPin1, true, true, inverted),
    CommonGPIO(lmswPin2, true, true, inverted)
}
{}

void LimitSwitches::firstInitInterrupts()
{
    for (auto &pin : this->limitSwitches)
    {
        const int pinNum = pin.getPin();
        gpio_set_irq_enabled_with_callback(pinNum, GPIO_IRQ_EDGE_FALL, true, &commonIsr);
    }
}

void LimitSwitches::setInterrupt(const int gpio, const bool interruptStatus)
{
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, interruptStatus);
}



