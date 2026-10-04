#include "encoder.h"

#include <iostream>

#include "hardware/gpio.h"
#include "interrupts/interrupts.h"

Encoder::Encoder(int pinA, int pinB, bool pullup)
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
        pinA->getPin(),
        GPIO_IRQ_EDGE_RISE,
        true,
        &commonIsr
    );
}

bool Encoder::processInterrupts()
{
    bool encoderEvent = false;
    bool eventReceived = false;

    while (queue_try_remove(&encEventQ, &encoderEvent))
    {
        updatePosition();
        eventReceived = true;
    }

    return eventReceived;
}

EncoderState Encoder::checkDirection() const
{
    if (pinB->read() == 0)
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
        ++encoderPosition;

        /*std::cout
            << "[ENCODER] Clockwise | position = "
            << encoderPosition
            << std::endl;
            */
    }
    else
    {
        --encoderPosition;
        /*
        std::cout
            << "[ENCODER] Counter-clockwise | position = "
            << encoderPosition
            << std::endl;
            */
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