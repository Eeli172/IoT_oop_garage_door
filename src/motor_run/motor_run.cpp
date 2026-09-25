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
    movementCalibration{0, 0, 0, 0, false},
    doorStuck(false)
{
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
    bool lmswMtrStop = false;
    bool lmswEncStop = false;
    bool sw1Stop = false;

    motorStepCounter = 0;
    doorStuck = false;

    const bool stuckDetectionEnabled =
        calibrated && currentDoorState == IN_BETWEEN;

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
        for (uint i = 0; i < MTR_COILS; i++)
            gpio_put(
                motorCoils[i]->getPin(),
                runInstructionsMatrix[step][i]);

        sleep_ms(1);
        updateStep(direction);
        motorStepCounter++;

        processEncoderEvents(encoder);

        if (stuckDetectionEnabled && checkForStuck(encoder))
        {
            handleDoorStuck(encoder);
            return false;
        }

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
    const int motorSteps =
        direction
            ? movementCalibration.motorStepsOpening
            : movementCalibration.motorStepsClosing;

    const int encoderSteps =
        direction
            ? movementCalibration.encoderStepsOpening
            : movementCalibration.encoderStepsClosing;

    if (motorSteps <= 0 || encoderSteps <= 0)
        return false;

    const float motorStepsPerEncoder =
        static_cast<float>(motorSteps) / encoderSteps;

    const int motorStepsNeeded =
        static_cast<int>(motorStepsPerEncoder * 2.0f);

    if (motorStepCounter - stuckCheck.motorSteps < motorStepsNeeded)
        return false;

    const int encoderMovement =
        std::abs(encoder.getPosition() - stuckCheck.encoderPosition);

    std::cout << "[STUCK CHECK] Motor: " << motorStepCounter - stuckCheck.motorSteps
        << " | Encoder: " << encoderMovement
        << " | Ratio: " << motorStepsPerEncoder << std::endl;

    resetStuckCheck(encoder);

    return encoderMovement == 0;
}

void Motor::resetStuckCheck(Encoder& encoder)
{
    stuckCheck.motorSteps = motorStepCounter;
    stuckCheck.encoderPosition = encoder.getPosition();
}

void Motor::handleDoorStuck(Encoder& encoder)
{
    stopMotor();
    doorStuck = true;
    errorState = true;
    calibrated = false;

    std::cout << "\n================================" << std::endl;
    std::cout << "ERROR: DOOR STUCK" << std::endl;
    std::cout << "Motor steps: " << motorStepCounter << std::endl;
    std::cout << "Encoder position: " << encoder.getPosition() << std::endl;
    std::cout << "================================\n" << std::endl;
}

void Motor::processEncoderEvents(Encoder& encoder)
{
    bool encoderEvent;

    while (queue_try_remove(&encEventQ, &encoderEvent))
        encoder.updatePosition();
}


void Motor::calibrateMotor(Encoder& encoder)
{
    calibrated = false;
    errorState = false;
    doorStuck = false;

    if (!gpio_get(PIN_LMSW_ENC))
    {
        runMotor(encoder);
    }
    else
    {
        direction = !direction;
    }

    sleep_ms(100);

    encoder.resetPosition();
    motorStepCounter = 0;

    runMotor(encoder);

    if (!direction)
    {
        movementCalibration.motorStepsOpening = motorStepCounter;
        movementCalibration.encoderStepsOpening =
            std::abs(encoder.getPosition());
    }
    else
    {
        movementCalibration.motorStepsClosing = motorStepCounter;
        movementCalibration.encoderStepsClosing =
            std::abs(encoder.getPosition());
    }

    std::cout << "\n[CALIBRATION] First run:" << std::endl;
    std::cout << "Motor steps: "
        << motorStepCounter << std::endl;
    std::cout << "Encoder steps: "
        << std::abs(encoder.getPosition()) << std::endl;

    encoder.resetPosition();
    motorStepCounter = 0;

    sleep_ms(100);

    runMotor(encoder);

    if (!direction)
    {
        movementCalibration.motorStepsOpening = motorStepCounter;
        movementCalibration.encoderStepsOpening =
            std::abs(encoder.getPosition());
    }
    else
    {
        movementCalibration.motorStepsClosing = motorStepCounter;
        movementCalibration.encoderStepsClosing =
            std::abs(encoder.getPosition());
    }

    std::cout << "\n[CALIBRATION] Second run:" << std::endl;
    std::cout << "Motor steps: "
        << motorStepCounter << std::endl;
    std::cout << "Encoder steps: "
        << std::abs(encoder.getPosition()) << std::endl;

    movementCalibration.valid =
        movementCalibration.motorStepsOpening > 0 &&
        movementCalibration.encoderStepsOpening > 0 &&
        movementCalibration.motorStepsClosing > 0 &&
        movementCalibration.encoderStepsClosing > 0;

    calibrated = movementCalibration.valid;
    errorState = !movementCalibration.valid;

    encoder.resetPosition();
    motorStepCounter = 0;
}


const Motor::MovementCalibration& Motor::getCalibration() const
{
    return movementCalibration;
}

bool Motor::isDoorStuck() const
{
    return doorStuck;
}

doorState_t Motor::getCurrentState() const
{
    return currentDoorState;
}

doorState_t Motor::getPreviousState() const
{
    return previousDoorState;
}

void Motor::setCurrentState(const doorState_t& dir)
{
    previousDoorState = currentDoorState;
    currentDoorState = dir;
}

void Motor::updateStep(bool opening)
{
    if (opening)
        step = step > 0 ? step - 1 : MTR_STEPS - 1;
    else
        step = step < MTR_STEPS - 1 ? step + 1 : 0;
}

bool Motor::getDirection()
{
    return direction;
}

bool Motor::getErrorState()
{
    return errorState;
}

void Motor::createStatusLeds()
{
    statusLeds = std::make_shared<StatusLeds>(
        std::vector{PIN_LED_1, PIN_LED_2, PIN_LED_3},
        weak_from_this());
}

void Motor::updateStatusLeds()
{
    statusLeds->updateCall();
}

void Motor::stopMotor()
{
    for (const auto& coil : motorCoils)
        coil->write(false);
}

void Motor::clearDoorStuck()
{
    doorStuck = false;
    errorState = false;
    calibrated = true;
}

void Motor::reverseDirection()
{
    direction = !direction;
}
