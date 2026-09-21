#include "encoder.h"
#include <iostream>
#include "hardware/gpio.h"

Encoder::Encoder(int pinA, int pinB, bool pullup)
{
    // Initialize encoder pins
    this->pinA = std::make_unique<CommonGPIO>(pinA, true, pullup, false);
    this->pinB = std::make_unique<CommonGPIO>(pinB, true, pullup, false);
}

void Encoder::initInterrupt() const
{

    gpio_set_irq_enabled_with_callback(
        this->pinA->getPin(),
        GPIO_IRQ_EDGE_RISE,
        true,
        &commonIsr
    );
}


//checks direction
EncoderState Encoder::checkDirection() const
{
    if (this->pinB->read() == 0)
    {
        return EncoderState::Clockwise;
    }
    return EncoderState::CounterClockwise;
}