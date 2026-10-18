#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H

/* STL */
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <string>

/* CUSTOM HEADERS */
#include "../gpios/gpios.h"
#include "../gpios/encoder/encoder.h"
#include "../../types/types.h"


/*
 *
 * ========================================================================
 * MOTOR MAIN CLASS
 * ========================================================================
 *
 */

class Motor
{
public:
    /**
     * @brief Initializes the motor GPIO coils and limit switches.
     * @param coilGpios Vector of GPIO pins connected to the motor coils.
     * @param inverted true to invert the GPIO output values, false otherwise.
     */
    Motor(const std::vector<int>& coilGpios, bool inverted);

    ~Motor() = default;

    Motor(const Motor&) = delete;
    Motor& operator=(const Motor&) = delete;


    // ========================================================================
    // MOTOR CONTROL
    // ========================================================================

    /**
     * @brief Runs the motor until stopped by a button, limit switch,
     * stuck detection, MQTT command, or calculated travel limit.
     * @param encoder Reference to the encoder used to track motor movement.
     * @param MqttStop Optional callback used to check for an MQTT stop command.
     * @return true if the motor was stopped manually by SW1 or an MQTT command.z
     * false if it stopped because of a limit switch, travel limit, or error.
     */
    bool runMotor(
        Encoder& encoder,
        const std::optional<std::function<bool(const std::string&)>>& MqttStop
    );

    /**
     * @brief Calibrates the motor movement and measures the motor-to-encoder
     * ratio. Attempts to leave the door in the closed position.
     * @param encoder Reference to the encoder used during calibration.
     */
    void calibrateMotor(Encoder& encoder);

    /**
     * @brief Recalibrates the estimated motor position when a limit switch
     * is activated.
     */
    void recalibrationOnTouch();


    // ========================================================================
    // MOTOR SETTERS
    // ========================================================================

    /**
     * @brief Sets the calibrated motor-steps-per-encoder-step ratio.
     * @param steps Number of motor steps per encoder step.
     */
    void setMtrStpsPerEncder(float steps);

    /**
     * @brief Sets the current motor movement direction.
     * @param direction true to move towards the encoder, false otherwise.
     */
    void setDirection(bool direction);

    /**
     * @brief Updates the motor calibration state.
     * @param calibState true if calibrated, false otherwise.
     */
    void setCalibState(const bool& calibState);

    /**
     * @brief Sets the calibrated number of motor steps between the two limits.
     * @param steps Total motor steps between the open and closed positions.
     */
    void setMotorStepsBetweenLimits(int steps);


    // ========================================================================
    // MOTOR GETTERS
    // ========================================================================

    /**
     * @brief Returns the calibrated motor-steps-per-encoder-step ratio.
     * @return Number of motor steps per encoder step.
     */
    [[nodiscard]] float getMtrStpsPerEncder() const;

    /**
     * @brief Returns the current motor movement direction.
     * @return true if moving towards the encoder, false otherwise.
     */
    [[nodiscard]] bool getDirection() const;

    /**
     * @brief Returns the current motor error state.
     * @return true if an error, such as a stuck door, has been detected.
     */
    [[nodiscard]] bool getErrorState() const;

    /**
     * @brief Returns the calibrated motor travel distance between the limits.
     * @return Total motor steps between the open and closed positions.
     */
    [[nodiscard]] int getMotorStepsBetweenLimits() const;


    // ========================================================================
    // MOTOR POSITION
    // ========================================================================

    /**
     * @brief Restores the estimated motor position, typically during startup.
     * @param steps Restored position in motor steps.
     * @param known true if the restored position is considered valid,
     * false otherwise.
     */
    void restorePosition(int steps, bool known);

private:
    // ========================================================================
    // MOTOR GPIO DEVICES
    // ========================================================================

    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::unique_ptr<Switches> limitSwitches;


    // ========================================================================
    // MOTOR VARIABLES
    // ========================================================================

    // Motor coil activation sequence: 8 steps and 4 coils.
    static int InstructionsMatrix[8][4];

    int step;
    int motorStepCounter;
    int motorStepsBetweenLimits;
    int currentPositionSteps;

    // Indicates whether the current motor position is known.
    bool positionKnown;

    // true == moving towards the encoder.
    bool direction;

    // true == door stuck or another motor error detected.
    bool errorState;

    // Indicates whether motor calibration has been completed.
    bool calibrated;

    // Counts motor steps since the last detected encoder movement.
    int stepsSinceEncoder;

    // Stores the motor-to-encoder calibration ratio and its validity.
    MovementCalibration_t movementCalibration;


    // ========================================================================
    // MOTOR CONTROL HELPERS
    // ========================================================================

    /**
     * @brief Disables all motor coil outputs.
     */
    void stopMotor();

    /**
     * @brief Reverses the current motor movement direction.
     */
    void reverseDirection();

    /**
     * @brief Determines whether stuck detection can be performed using
     * valid motor calibration data.
     * @return true if stuck detection is available, false otherwise.
     */
    [[nodiscard]] bool stuckDetectionEnabled() const;

    /**
     * @brief Calculates the number of motor steps per encoder step.
     * @param motorSteps Total motor steps measured during calibration.
     * @param encoderSteps Total encoder steps measured during calibration.
     * @return Motor-steps-per-encoder-step ratio, or 0 if encoderSteps
     * is less than or equal to zero.
     */
    float calculateMotorStepsPerEncoder(
        int motorSteps,
        int encoderSteps
    ) const;

    /**
     * @brief Applies the current motor coil sequence and advances
     * to the next step.
     */
    void runMotorSteps();

    /**
     * @brief Checks whether the motor has stopped moving by comparing
     * motor steps with the expected encoder movement.
     */
    void checkForStuck();

    /**
     * @brief Checks whether the next motor step would exceed the
     * calibrated travel limits.
     * @param directionAtStart Movement direction at the beginning
     * of the current motor run.
     * @return true if the next step would exceed a known travel limit,
     * false otherwise.
     */
    [[nodiscard]] bool checkPositionLimit(bool directionAtStart) const;

    /**
     * @brief Finalizes the estimated motor position after movement stops.
     * @param directionAtStart Movement direction at the beginning
     * of the current motor run.
     * @param lmswMtrStop true if the motor-side limit switch was triggered.
     * @param lmswEncStop true if the encoder-side limit switch was triggered.
     */
    void finalizeMotorPosition(
        bool directionAtStart,
        bool lmswMtrStop,
        bool lmswEncStop
    );
};


#endif // IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
