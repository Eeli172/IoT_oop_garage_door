#include "motor_run.h"

#include <iostream>
#include <cmath>

#include "../interrupts/interrupts.h"
#include "../macros/macros.h"
#include "../gpios/leds/leds.h"

#include "pico/util/queue.h"
#include "hardware/gpio.h"

Motor::Motor(const std::vector<int>& coilGpios, bool inverted) :
    currentDoorState(CLOSED),
    stepsToEnd(0),
    step(0),
    direction(true),
    errorState(false),
    calibrated(false),
    motorStepCounter(0),
    stuckCheck{0, 0},
    movementCalibration{0.0f, false},
    doorStuck(false)
{
    // Create GPIO objects for the motor coils.
    for (const auto& coilPin : coilGpios)
        motorCoils.push_back(std::make_unique<CommonGPIO>(
            coilPin, false, false, inverted));
}

int Motor::runInstructionsMatrix[8][4] =
{
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

bool Motor::runMotor(Encoder& encoder)
{
    // Local flags used to stop the motor.
    bool lmswMtrStop = false;
    bool lmswEncStop = false;
    bool sw1Stop = false;

    motorStepCounter = 0;
    doorStuck = false;

    // Choose a direction away from an already pressed limit switch.
    // Correct direction when starting on an active limit switch.

    // Enable stuck detection for calibrated movements.
    const bool stuckDetectionEnabled = calibrated;

    resetStuckCheck(encoder);

    std::cout << "\n[MOTOR] Starting movement" << std::endl;
    std::cout << "[MOTOR] Direction: "
        << (direction ? "OPENING" : "CLOSING")
        << std::endl;
    std::cout << "[MOTOR] Stuck detection: "
        << (stuckDetectionEnabled ? "ON" : "OFF")
        << std::endl;

    while (((direction && !lmswEncStop) ||
        (!direction && !lmswMtrStop)) && !sw1Stop)
    {
        // Apply the current step pattern to the motor coils.
        for (uint i = 0; i < MTR_COILS; i++)
            gpio_put(
                motorCoils[i]->getPin(),
                runInstructionsMatrix[step][i]);

        sleep_ms(1);
        updateStep(direction);
        motorStepCounter++;

        // Update encoder position from queued encoder events.
        processEncoderEvents(encoder);

        // Start checking for a stuck door after the startup delay.
        if (stuckDetectionEnabled &&
            motorStepCounter > 1000 &&
            checkForStuck(encoder))
        {
            handleDoorStuck(encoder);
            return false;
        }

        // Check for user or limit-switch stop events.
        queue_try_remove(&sw1EventQ, &sw1Stop);
        queue_try_remove(&lmswMtrEventQ, &lmswMtrStop);
        queue_try_remove(&lmswEncEventQ, &lmswEncStop);
    }

    stopMotor();
    direction = !direction;

    std::cout << "[MOTOR] Movement finished" << std::endl;
    std::cout << "[MOTOR] Motor steps: " << motorStepCounter << std::endl;
    std::cout << "[MOTOR] Encoder position: " << encoder.getPosition() << std::endl;

    return sw1Stop;
}

bool Motor::checkForStuck(Encoder& encoder)
{
    // Use the single calibrated motor/encoder ratio.
    const float motorStepsPerEncoder =
        movementCalibration.motorStepsPerEncoder;

    if (motorStepsPerEncoder <= 0.0f)
        return false;

    const int motorStepsNeeded =
        static_cast<int>(motorStepsPerEncoder * 2.0f);

    // Wait until enough motor steps have passed before checking.
    if (motorStepCounter - stuckCheck.motorSteps < motorStepsNeeded)
        return false;

    const int encoderMovement =
        std::abs(encoder.getPosition() - stuckCheck.encoderPosition);

    std::cout << "[STUCK CHECK] Motor: "
        << motorStepCounter - stuckCheck.motorSteps
        << " | Encoder: "
        << encoderMovement
        << " | Ratio: "
        << motorStepsPerEncoder
        << std::endl;

    resetStuckCheck(encoder);

    // No encoder movement means the door is considered stuck.
    return encoderMovement == 0;
}

void Motor::resetStuckCheck(Encoder& encoder)
{
    // Store the current motor and encoder positions as a reference.
    stuckCheck.motorSteps = motorStepCounter;
    stuckCheck.encoderPosition = encoder.getPosition();
}

void Motor::handleDoorStuck(Encoder& encoder)
{
    // Stop the motor and set the error state.
    stopMotor();
    doorStuck = true;
    errorState = true;

    std::cout << "\n================================" << std::endl;
    std::cout << "ERROR: DOOR STUCK" << std::endl;
    std::cout << "Motor steps: " << motorStepCounter << std::endl;
    std::cout << "Encoder position: " << encoder.getPosition() << std::endl;
    std::cout << "================================\n" << std::endl;
}

void Motor::processEncoderEvents(Encoder& encoder)
{
    // Process all pending encoder events.
    bool encoderEvent;

    while (queue_try_remove(&encEventQ, &encoderEvent))
        encoder.updatePosition();
}

void Motor::calibrateMotor(Encoder& encoder)
{
    // Reset states before starting calibration.
    calibrated = false;
    errorState = false;
    doorStuck = false;

    // Reverse direction if the limit switch is already pressed.
    if (!gpio_get(PIN_LMSW_MTR))
    {
        direction = !direction;
    }
    else
    {
        runMotor(encoder);
    }

    sleep_ms(100);

    float ratio[2];

    // Run calibration twice and calculate one ratio from each run.
    for (int i = 0; i < 2; i++)
    {
        encoder.resetPosition();
        motorStepCounter = 0;

        runMotor(encoder);

        const int encoderSteps = std::abs(encoder.getPosition());

        ratio[i] = encoderSteps > 0
                       ? static_cast<float>(motorStepCounter) / encoderSteps
                       : 0.0f;

        std::cout << "[CALIBRATION] Run " << i + 1
            << " | Motor: " << motorStepCounter
            << " | Encoder: " << encoderSteps
            << " | Ratio: " << ratio[i]
            << std::endl;

        sleep_ms(100);
    }

    // Use the average of the two calibration ratios.
    movementCalibration.motorStepsPerEncoder =
        (ratio[0] + ratio[1]) / 2.0f;

    movementCalibration.valid =
        ratio[0] > 0.0f &&
        ratio[1] > 0.0f;

    calibrated = movementCalibration.valid;
    errorState = !movementCalibration.valid;

    encoder.resetPosition();
    motorStepCounter = 0;

    std::cout << "[CALIBRATION] Average ratio: "
        << movementCalibration.motorStepsPerEncoder
        << std::endl;
}

const Motor::MovementCalibration& Motor::getCalibration() const
{
    // Return the current calibration data.
    return movementCalibration;
}

bool Motor::isDoorStuck() const
{
    // Return whether the motor detected a stuck door.
    return doorStuck;
}

doorState_t Motor::getCurrentState() const
{
    // Return the current door state.
    return currentDoorState;
}

doorState_t Motor::getPreviousState() const
{
    // Return the previous door state.
    return previousDoorState;
}

void Motor::setCurrentState(const doorState_t& dir)
{
    // Save the old state before changing it.
    previousDoorState = currentDoorState;
    currentDoorState = dir;
}

void Motor::updateStep(bool opening)
{
    // Move through the coil sequence in the selected direction.
    if (opening)
        step = step > 0 ? step - 1 : MTR_STEPS - 1;
    else
        step = step < MTR_STEPS - 1 ? step + 1 : 0;
}

bool Motor::getDirection()
{
    // Return the current motor direction.
    return direction;
}

bool Motor::getErrorState()
{
    // Return whether an error is active.
    return errorState;
}

void Motor::createStatusLeds()
{
    // Create the status LED controller.
    statusLeds = std::make_shared<StatusLeds>(
        std::vector{PIN_LED_1, PIN_LED_2, PIN_LED_3},
        weak_from_this());
}

void Motor::updateStatusLeds()
{
    // Update the motor status LEDs.
    statusLeds->updateCall();
}

void Motor::stopMotor()
{
    // Turn off all motor coils.
    for (const auto& coil : motorCoils)
        coil->write(false);
}

void Motor::clearDoorStuck()
{
    // Clear the stuck and error flags.
    doorStuck = false;
    errorState = false;
}

void Motor::reverseDirection()
{
    // Toggle the motor direction.
    direction = !direction;
}
bool Motor::moveAwayFromLimit()
{
    // Select a safe direction when a limit is already pressed.
    if (!gpio_get(PIN_LMSW_MTR))
    {
        reverseDirection();
    }

    if (!gpio_get(PIN_LMSW_ENC))
    {
        reverseDirection();
    }

    return false;
}