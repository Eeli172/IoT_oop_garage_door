/* STL */
#include <memory>
#include <iostream>

/* CUSTOM */
#include "../macros/macros.h"
#include "pico/stdlib.h" // IWYU pragma: keep
#include "gpios.h"

#include "../interrupts/interrupts.h"


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


// SWITCHES
Switches::Switches(const std::vector<int> &pins, bool input, bool pullup, bool inverted) {
    for (const auto &pin : pins) {
        switches.push_back(std::make_unique<CommonGPIO>(pin, input, pullup, inverted));
    }
}

// Allows toggling of multiple interrupts of multiple gpios at a time
void Switches::enableInterrupts(const std::vector<int> &swPins) const
{
    if (swPins.size() > SW_AMOUNT){
        std::cerr << "System does not support over [ "<< SW_AMOUNT <<" ] per class." << std::endl;
        return;
    }

    for (std::size_t i = 0; i < swPins.size(); i++) {
        for (const auto &swGpio: this->switches) {
            if (swGpio->getPin() == swPins[i]) {
                gpio_set_irq_enabled(swPins[i], GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
            }
        }
    }
}

void Switches::initInterrupts(const std::vector<int> &swPins, const std::vector<bool> &initStates)
{
    if (swPins.size() != initStates.size())
    {
        std::cerr << "[ERROR] --> Vector sizes don't match" << std::endl;
        return;
    }

    for (size_t i = 0; i < swPins.size(); i++)
    {
        for (const auto &swGpio : this->switches)
        {
            if (swPins[i] == swGpio->getPin())
            {
                gpio_set_irq_enabled_with_callback(swPins[i], GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, initStates[i], &commonIsr);
                break;
            }
        }
    }
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



