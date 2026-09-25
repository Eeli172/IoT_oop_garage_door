#ifndef GARAGE_DOOR_ENCODER_H
#define GARAGE_DOOR_ENCODER_H

#include "pico/stdlib.h"
#include <memory>

#include "../gpios.h"
#include "../../interrupts/interrupts.h"
#include "../../macros/macros.h"

enum class EncoderState
{
    Clockwise,
    CounterClockwise,
    Idle,
    Brutestop
};

class Encoder
{
public:
    Encoder(int pinA, int pinB, bool pullup);
    void enableInterrupts() const;
    Encoder(const Encoder &) = delete;
    ~Encoder() = default;

    void initInterrupt() const;

    EncoderState checkDirection() const;

    void updatePosition();

    void resetPosition();

    int getPosition() const;

private:
    std::unique_ptr<CommonGPIO> pinA;
    std::unique_ptr<CommonGPIO> pinB;

    int encoderPosition;
};

#endif // GARAGE_DOOR_ENCODER_H