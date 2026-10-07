#include "encoder.h"
#include "hardware/gpio.h"
#include "interrupts/interrupts.h"


/* ########## P U B L I C ########## */

/* ENCODER INITIALIZATION */

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


/* ENCODER INTERRUPTS */

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


/* ENCODER DIRECTION */

EncoderState Encoder::checkDirection() const
{
    if (pinB->read() == 0)
    {
        return EncoderState::Clockwise;
    }

    return EncoderState::CounterClockwise;
}


/* ENCODER POSITION */
//uncomment prints to check the direction and position of the encoder could be used in mqtt

void Encoder::updatePosition()
{
    const EncoderState direction = checkDirection();

    if (direction == EncoderState::Clockwise)
    {
        ++encoderPosition;

        /* std::cout << "[ENCODER] Clockwise | position = "
                  << encoderPosition << std::endl; */
    }
    else
    {
        --encoderPosition;
        /* std::cout << "[ENCODER] Counter-clockwise | position = "
         * << encoderPosition << std::endl; */
    }
}
//used in motor calibration
void Encoder::resetPosition()
{
    encoderPosition = 0;

    /* std::cout << "[ENCODER] Position reset to 0" << std::endl; */
}

// used in motor calibration to calculate ratio= (motorsteps/encoder steps)
int Encoder::getPosition() const
{
    return encoderPosition;
}