#include <iostream>
#include "pico/stdlib.h"
#include "gpios.h"
#include <memory>

CommonGPIO::CommonGPIO(const int pin, const bool input, const bool pullup, const bool inverted) :
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





