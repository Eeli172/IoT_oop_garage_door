#include "pico/stdlib.h" // IWYU pragma: keep
#include "gpios.h"
#include <memory>
#include <array>

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
Switches::Switches(std::vector<int> pins, bool input, bool pullup, bool inverted) {
    for (auto pin : pins) {
        switches.push_back(std::make_unique<CommonGPIO>(pin, input, pullup, inverted));
    }
}

void Switches::firstInitInterrupts()
{
    for (auto &sw : this->switches)
    {
        const int pinNum = sw->getPin();
        gpio_set_irq_enabled_with_callback(pinNum, GPIO_IRQ_EDGE_FALL, true, &commonIsr);
    }
}




// LEDS
StatusLeds::StatusLeds(std::array<int, TOTAL_LEDS>& ledGpios, bool inverted)
{

}

void StatusLeds::writeLed (const bool &ledState, const int ledGpio) const
{
    for (const auto &pin : this->ledGpios)
    {
        if (pin->getPin() == ledGpio)
        {
            pin->write(ledState);
            break;
        }
    }
}



