#include "encoder.h"
#include "hardware/gpio.h"
#include <iostream>

Encoder::Encoder(int pinA, int pinB, bool pullup) :
    encoderPosition(0)
{
    this->pinA = std::make_unique<CommonGPIO>(
        pinA,
        true,
        pullup,
        false
    );

    this->pinB = std::make_unique<CommonGPIO>(
        pinB,
        true,
        pullup,
        false
    );
}


void Encoder::enableInterrupts() const
{
    gpio_set_irq_enabled_with_callback(
        this->pinA->getPin(),
        GPIO_IRQ_EDGE_RISE,
        true,
        &commonIsr
    );
}

EncoderState Encoder::checkDirection() const
{
    if (this->pinB->read() == 0)
    {
        return EncoderState::Clockwise;
    }

    return EncoderState::CounterClockwise;
}

void Encoder::updatePosition()
{
    const EncoderState direction = checkDirection();

    if (direction == EncoderState::Clockwise)
    {
        encoderPosition++;

        std::cout << "[ENCODER] Clockwise | position = " << encoderPosition << std::endl;
    }
    else if (direction == EncoderState::CounterClockwise)
    {
        encoderPosition--;

        std::cout << "[ENCODER] Counter-clockwise | position = " << encoderPosition << std::endl;
    }
}

void Encoder::resetPosition()
{
    encoderPosition = 0;

    std::cout << "[ENCODER] Position reset to 0" << std::endl;
}

int Encoder::getPosition() const
{
    return encoderPosition;
}
