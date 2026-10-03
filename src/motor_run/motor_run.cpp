#include "motor_run.h"

#include <cmath>
#include <iostream>

#include "../interrupts/interrupts.h"
#include "../macros/macros.h"

#include "hardware/gpio.h"
#include "pico/util/queue.h"


Motor::Motor(
    const std::vector<int>& coilGpios,
    bool inverted) :
    step(0),
    motorStepCounter(0),
    direction(true),
    errorState(false), // Yhistä errorstate & door stuck muuttujat samaksi ?
    calibrated(false),
    doorStuck(false), // Yhistä errorstate & door stuck muuttujat samaksi ?
    movementCalibration{0.0f, false}
{
    for (const auto& coilPin : coilGpios)
    {
        motorCoils.push_back(
            std::make_unique<CommonGPIO>(
                coilPin,
                false,
                false,
                inverted));
    }
}

int Motor::InstructionsMatrix[8][4] =
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


void Motor::runMotorSteps()
{
    // Apply current coil pattern
    for (uint i = 0; i < MTR_COILS; ++i)
    {
        gpio_put(
            motorCoils[i]->getPin(),
            InstructionsMatrix[step][i]);
    }

    // Move to next step
    if (direction)
    {
        step = step > 0 ? step - 1 : MTR_STEPS - 1;
    }
    else
    {
        step = step < MTR_STEPS - 1 ? step + 1 : 0;
    }
}

bool Motor::reverseOnPressedLimitSwitch()
{
    // Closing while already at closing limit
    if (direction && !gpio_get(PIN_LMSW_MTR))
    {
        reverseDirection();
        return true;
    }

    // Opening while already at opening limit
    if (!direction && !gpio_get(PIN_LMSW_ENC))
    {
        reverseDirection();
        return true;
    }

    return false;
}

void Motor::reverseDirection()
{
    direction = !direction;
}


bool Motor::getDirection() const
{
    return direction;
}


bool Motor::runMotor(Encoder& encoder)
{
    bool lmswMtrStop = false;
    bool lmswEncStop = false;
    bool sw1Stop = false;

    motorStepCounter = 0;
    stepsSinceEncoder = 0;
    doorStuck = false;
    errorState = false;

    reverseOnPressedLimitSwitch();

    while (((direction && !lmswEncStop) || (!direction && !lmswMtrStop)) && !sw1Stop && !errorState)
    {
        runMotorSteps();

        sleep_ms(MOTOR_STEP_DELAY_MS);

        ++motorStepCounter;
        ++stepsSinceEncoder;

        if (encoder.processInterrupts())
        {
            stepsSinceEncoder = 0;
        }

        if (stuckDetectionEnabled())
        {
            checkForStuck();
        }

        queue_try_remove(&sw1EventQ, &sw1Stop);
        queue_try_remove(&lmswMtrEventQ, &lmswMtrStop);
        queue_try_remove(&lmswEncEventQ, &lmswEncStop);
    }

    stopMotor();

    if (errorState)
    {
        doorStuck = true;
        return false;
    }

    return sw1Stop;
}


void Motor::stopMotor()
{
    for (const auto& coil : motorCoils)
    {
        coil->write(false);
    }
}


void Motor::calibrateMotor(Encoder& encoder)
{
    calibrated = false;
    errorState = false;
    doorStuck = false;

    float totalRatio = 0.0f;
    bool calibrationValid = true;

    for (int run = 0; run < CALIBRATION_RUNS; ++run)
    {
        encoder.resetPosition();
        motorStepCounter = 0;

        runMotor(encoder);

        const int encoderSteps = std::abs(encoder.getPosition());

        const float ratio = calculateMotorStepsPerEncoder( motorStepCounter, encoderSteps);

        totalRatio += ratio;

        if (ratio <= 0.0f)
        {
            calibrationValid = false;
        }

        std::cout
            << "[CALIBRATION] Run "
            << run + 1
            << " | Motor: "
            << motorStepCounter
            << " | Encoder: "
            << encoderSteps
            << " | Ratio: "
            << ratio
            << std::endl;
    }

    movementCalibration.motorStepsPerEncoder = totalRatio / CALIBRATION_RUNS;

    movementCalibration.valid = calibrationValid && movementCalibration.motorStepsPerEncoder > 0.0f;

    calibrated = movementCalibration.valid;

    errorState = !movementCalibration.valid;

    encoder.resetPosition();
    motorStepCounter = 0;

    std::cout
        << "[CALIBRATION] Average ratio: "
        << movementCalibration.motorStepsPerEncoder
        << std::endl;
}




float Motor::calculateMotorStepsPerEncoder(
    int motorSteps,
    int encoderSteps) const
{
    if (encoderSteps <= 0)
    {
        return 0.0f;
    }

    return static_cast<float>(motorSteps) / encoderSteps;
}



bool Motor::stuckDetectionEnabled() const
{
    return calibrated && movementCalibration.motorStepsPerEncoder > 0.0f;
}


void Motor::checkForStuck()
{
    const int expectedMotorSteps = static_cast<int>(std::ceil(movementCalibration.motorStepsPerEncoder));

    const int allowedMotorSteps = expectedMotorSteps + STUCK_STEP_TOLERANCE;

    if (stepsSinceEncoder > allowedMotorSteps)
    {
        std::cout
            << "[STUCK CHECK] Door appears stuck"
            << " | Steps since encoder: " << stepsSinceEncoder
            << " | Allowed: " << allowedMotorSteps
            << std::endl;

        errorState = true;
    }
}



//used in main and sets steps to 0
void Motor::clearDoorStuck()
{
    doorStuck = false;
    errorState = false;
    stepsSinceEncoder = 0;
}



// Errors and led
bool Motor::getErrorState() const
{
    return errorState;
}


// Door stuck status
bool Motor::isDoorStuck() const
{
    return doorStuck;
}