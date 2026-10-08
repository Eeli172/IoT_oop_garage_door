#ifndef GARAGE_DOOR_ENCODER_H
#define GARAGE_DOOR_ENCODER_H

/* STL */
#include <memory>

/* CUSTOM */
#include "../gpios.h"


/* ENCODER STATE */

enum class EncoderState
{
    Clockwise,
    CounterClockwise
};


/* ENCODER MAIN CLASS */

class Encoder
{
public:
    Encoder(int pinA, int pinB, bool pullup);
    ~Encoder() = default;

    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;


    /* INTERRUPTS */
    void enableInterrupts() const;
    bool processInterrupts();


    /* DIRECTION */
    EncoderState checkDirection() const;

    /* POSITION */
    void updatePosition();
    void resetPosition();
    int getPosition() const;

private:
    /* ENCODER GPIO PINS */
    std::unique_ptr<CommonGPIO> pinA;
    std::unique_ptr<CommonGPIO> pinB;

    /* ENCODER STATE */
    int encoderPosition = 0;
};

#endif // GARAGE_DOOR_ENCODER_H