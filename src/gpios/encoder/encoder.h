#ifndef GARAGE_DOOR_ENCODER_H
#define GARAGE_DOOR_ENCODER_H

#include <memory>
#include "pico/stdlib.h"
#include "../gpios.h"

enum class EncoderState
{
    Clockwise,
    CounterClockwise
};

class Encoder
{
public:
    Encoder(int pinA, int pinB, bool pullup);
    ~Encoder() = default;

    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;

    void enableInterrupts() const;
    bool processInterrupts();

    EncoderState checkDirection() const;

    void updatePosition();
    void resetPosition();

    int getPosition() const;

    void setPosition(int position);


private:
    std::unique_ptr<CommonGPIO> pinA;
    std::unique_ptr<CommonGPIO> pinB;

    int encoderPosition = 0;
};

#endif // GARAGE_DOOR_ENCODER_H