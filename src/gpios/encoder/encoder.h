#ifndef GARAGE_DOOR_ENCODER_H
#define GARAGE_DOOR_ENCODER_H

/* STL */
#include <memory>

/* CUSTOM */
#include "../gpios.h"


/*
 *
 * ========================================================================
 * ENCODER STATE
 * ========================================================================
 *
 */

/*
 * Defines the possible directions of encoder rotation.
 */
enum class EncoderState
{
    Clockwise,
    CounterClockwise
};


/*
 *
 * ========================================================================
 * ENCODER MAIN CLASS
 * ========================================================================
 *
 */

class Encoder
{
public:

    /**
     * @brief Initializes the encoder using two GPIO pins and the specified pullup configuration.
     * @param pinA GPIO pin used as the first encoder signal.
     * @param pinB GPIO pin used as the second encoder signal.
     * @param pullup true to enable pullup resistors, false otherwise.
     */
    Encoder(int pinA, int pinB, bool pullup);

    ~Encoder() = default;

    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;


    // ========================================================================
    // INTERRUPTS
    // ========================================================================

    /**
     * @brief Enables interrupts for the encoder GPIO pins.
     */
    void enableInterrupts() const;

    /**
     * @brief Processes pending encoder interrupt events.
     * @return true if an interrupt event was processed successfully, false otherwise.
     */
    bool processInterrupts();


    // ========================================================================
    // DIRECTION
    // ========================================================================

    /**
     * @brief Determines the current direction of encoder rotation.
     * @return EncoderState indicating clockwise or counterclockwise rotation.
     */
    EncoderState checkDirection() const;


    // ========================================================================
    // POSITION
    // ========================================================================

    /**
     * @brief Updates the encoder position based on the detected movement.
     */
    void updatePosition();

    /**
     * @brief Resets the encoder position to zero.
     */
    void resetPosition();

    /**
     * @brief Returns the current encoder position.
     * @return The current encoder position as an integer.
     */
    [[nodiscard]] int getPosition() const;


private:

    // ========================================================================
    // ENCODER GPIO PINS
    // ========================================================================

    std::unique_ptr<CommonGPIO> pinA;
    std::unique_ptr<CommonGPIO> pinB;


    // ========================================================================
    // ENCODER VARIABLES
    // ========================================================================

    int encoderPosition = 0;
};

#endif // GARAGE_DOOR_ENCODER_H