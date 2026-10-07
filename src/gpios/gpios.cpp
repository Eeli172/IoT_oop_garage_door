/* STL */
#include <memory>
#include <iostream>

/* CUSTOM */
#include "../macros/macros.h"
#include "pico/stdlib.h" // IWYU pragma: keep
#include "gpios.h"



/* GPIO INITIALIZATION */
CommonGPIO::CommonGPIO(const int pin, const bool input = true, const bool pullup = true, const bool inverted = false) :
pin(pin), input(input), is_dormant(true)
{
    gpio_init(pin);
    /* VALIDATE GPIO PIN */
    if (pin < 0 || pin > 29)
    {
        return;
    }
    /* RESERVE GPIO PIN */

    if ( const uint32_t mask = 1U << pin; (pins_in_use & mask) == 0)
    {
        pins_in_use |= mask;
        this->is_dormant = false;

        /* CONFIGURE GPIO DIRECTION */
        if (!input)
        {
            gpio_set_dir(pin, GPIO_OUT);
        }

        else
        {
            gpio_set_dir(pin, GPIO_IN);
        }

        /* CONFIGURE GPIO PULL-UP AND INPUT INVERSION */
        if (input && pullup && inverted)
        {
            gpio_pull_up(pin);
            gpio_set_inover(pin, GPIO_OVERRIDE_INVERT);
        }
    }
}

/* GPIO CLEANUP */
uint32_t CommonGPIO::pins_in_use = 0;

CommonGPIO::~CommonGPIO()
{
    if (!this->is_dormant) {
        /* RESET GPIO OVERRIDES AND PULL SETTINGS */
        gpio_set_outover(this->pin, GPIO_OVERRIDE_NORMAL);
        gpio_set_inover(this->pin, GPIO_OVERRIDE_NORMAL);
        gpio_disable_pulls(this->pin);

        /* RELEASE GPIO PIN */
        pins_in_use &= ~(1U << this->pin);
    }
}

/* GPIO STATUS */
CommonGPIO::operator bool() const
{
    return !this->is_dormant;
}

/* GPIO INPUT */
bool CommonGPIO::read() const
{
    if (this->is_dormant)
    {
        return false;
    }

    return gpio_get(this->pin);
}

/* GPIO OUTPUT */
void CommonGPIO::write(const bool value) const
{
    if (this->is_dormant)
    {
        return;
    }

    gpio_put(this->pin, value);
}

/* GPIO INFORMATION */
int CommonGPIO::getPin()
{
    if (this->is_dormant)
    {
        return -1;
    }
    return this->pin;
}


/* SWITCHES */
/* SWITCH INITIALIZATION */
Switches::Switches(const std::vector<int> &pins, bool input, bool pullup, bool inverted) {
    for (const auto &pin : pins) {
        switches.emplace_back(std::make_unique<CommonGPIO>(pin, input, pullup, inverted));
    }
}
/* SWITCH INTERRUPTS */

/* ALLOWS TOGGLING OF MULTIPLE INTERRUPTS OF MULTIPLE GPIOS AT A TIME*/
void Switches::setInterrupt(const std::vector<int> &swPins, const bool& enable) const
{
    if (swPins.size() > SW_AMOUNT){
        std::cerr << "System does not support over [ "<< SW_AMOUNT <<" ] per class." << std::endl;
        return;
    }

    for (std::size_t i = 0; i < swPins.size(); i++) {
        for (const auto &swGpio: this->switches) {
            if (swGpio->getPin() == swPins[i]) {
                gpio_set_irq_enabled(swPins[i], GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, enable);
            }
        }
    }
}
