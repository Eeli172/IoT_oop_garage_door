#include "motor_run.h"

#include <cmath>
#include <functional>
#include <iostream>

#include "../interrupts/interrupts.h"
#include "../macros/macros.h"

#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "types/types.h"


/* ########## P U B L I C ########## */

/* MOTOR INITIALIZATION */
Motor::Motor(
    const std::vector<int>& coilGpios,
    bool inverted) :
    step(0),
    motorStepCounter(0),
    motorStepsBetweenLimits(0),
    currentPositionSteps(0),
    positionKnown(false),
    direction(true),
    errorState(false),
    calibrated(false),
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
    limitSwitches = std::make_unique<Switches>(
        std::vector{PIN_LMSW_MTR_OPEN, PIN_LMSW_ENC_CLOSED}, true, true, true
    );
}


/* 
Run motor until stopped by:
    1. sw1 
    2. limit switches
    3. stuck detection
    4. mqtt command
Return: true (stopped manually (sw1 || mqtt command))  ||  false (otherwise)
*/
bool Motor::runMotor(Encoder& encoder, const std::optional<std::function<bool(const std::string&)>>& MqttStop)
{
    bool lmswMtrStop = false;
    bool lmswEncStop = false;
    bool manualStop = false;
    bool stoppedByPositionLimit = false;
    // Remember the starting direction so motor position tracking stays correct after reversing.
    const bool directionAtStart = direction;


    /* Reset movement state */
    motorStepCounter = 0;
    stepsSinceEncoder = 0;
    errorState = false;

    // Update motor position if a limit switch is active
    recalibrationOnTouch();

    limitSwitches->setInterrupt(std::vector{PIN_LMSW_MTR_OPEN, PIN_LMSW_ENC_CLOSED}, true);

    while (((direction && !lmswEncStop) ||
            (!direction && !lmswMtrStop))
        && !manualStop
        && !errorState)
    {
        queue_try_remove(&sw1EventQ, &manualStop);

        if (manualStop || (MqttStop.has_value() && MqttStop.value()("STOP")))
        {
            manualStop = true;
            break;
        }

        // Check travel limit before moving the next step
        if (checkPositionLimit(directionAtStart))
        {
            stoppedByPositionLimit = true;
            break;
        }

        /* RUN ONE MOTOR STEP */
        runMotorSteps();

        sleep_us(MOTOR_STEP_DELAY_US);

        /* TRACK MOTOR AND ENCODER MOVEMENT */
        ++motorStepCounter;
        ++stepsSinceEncoder;

        // Update estimated position once for each commanded motor step.
        if (positionKnown)
        {
            currentPositionSteps += directionAtStart ? 1 : -1;
        }


        /* RESET ENCODER STEP COUNTER WHEN ENCODER MOVES */
        if (encoder.processInterrupts())
        {
            stepsSinceEncoder = 0;
        }

        if (stuckDetectionEnabled())
        {
            checkForStuck();
        }
        // Fallback when the exact position is unknown.
        // This limits one run, but cannot establish the true door position.
        if (!positionKnown &&
            motorStepsBetweenLimits > 0 &&
            motorStepCounter >= motorStepsBetweenLimits)
        {

            errorState = true;
        }

        /* DISABLE LIMIT SWITCH INTERRUPTS */
        queue_try_remove(&lmswMtrEventQ, &lmswMtrStop);
        queue_try_remove(&lmswEncEventQ, &lmswEncStop);
    }

    limitSwitches->setInterrupt(std::vector{PIN_LMSW_MTR_OPEN, PIN_LMSW_ENC_CLOSED}, false);


    // Finalize position before reversing direction
    finalizeMotorPosition(directionAtStart, lmswMtrStop, lmswEncStop);

    //change direction and then stop
    this->reverseDirection();

    stopMotor();

    if (errorState)
    {
        positionKnown = false;
        return false;
    }

     if (stoppedByPositionLimit)
    {
        // The calculated endpoint was reached successfully.
        if (directionAtStart)
        {
            // CLOSING: endpoint is CLOSED.
            currentPositionSteps = motorStepsBetweenLimits;
        }
        else
        {
            // OPENING: endpoint is OPEN.
            currentPositionSteps = 0;
        }

        positionKnown = true;

    }

    return manualStop;
}


/*
Calibrate motor, a door is closed after calibration.
*/
void Motor::calibrateMotor(Encoder& encoder)
{
    this->direction = false;
    if (gpio_get(PIN_LMSW_MTR_OPEN)) this->direction = true;
    /* RESET MEASUREMENT COUNTERS */
    calibrated = false;
    errorState = false;

    int totalMotorSteps = 0;
    int totalEncoderSteps = 0;


    motorStepsBetweenLimits = 0;
    currentPositionSteps = 0;
    positionKnown = false;

    bool reachedMtr(false);

    for (int run = 0; run < 2; ++run)
    {
        encoder.resetPosition();
        motorStepCounter = 0;

        const bool startedAtMotorLimit =
            gpio_get(PIN_LMSW_MTR_OPEN);

        const bool measuringLimitDistance =
            startedAtMotorLimit && direction;



        runMotor(encoder, std::nullopt);



        /* RUN CALIBRATION IN BOTH DIRECTIONS */

        if (gpio_get(PIN_LMSW_MTR_OPEN))
        {
            currentPositionSteps = 0;
            positionKnown = true;
            reachedMtr = true;
        }

        // Allow the limit-switch signal to settle after motor movement stops.
        // Without this delay the GPIO may be read before the switch state is stable,
        // causing the calibration travel distance to remain invalid.
        sleep_ms(10);

        if (measuringLimitDistance &&
            !errorState &&
            gpio_get(PIN_LMSW_ENC_CLOSED))
        {
            motorStepsBetweenLimits = motorStepCounter;
            currentPositionSteps = motorStepsBetweenLimits;
            positionKnown = true;

        }

        totalMotorSteps += motorStepCounter;
        totalEncoderSteps += std::abs(encoder.getPosition());

        /* RUN CALIBRATION IN BOTH DIRECTIONS */


        /* STOP AT ENCODER LIMIT SWITCH = CLOSED STATE */
        if (reachedMtr == true && gpio_get(PIN_LMSW_ENC_CLOSED))
        {
            break;
        }
    }


    /* CALCULATE AND STORE MOTOR-TO-ENCODER RATIO*/
    /* AVERAGE RATIO 205-210*/
    const float ratio =
        calculateMotorStepsPerEncoder(
            totalMotorSteps,
            totalEncoderSteps);
    movementCalibration.motorStepsPerEncoder = ratio;
    movementCalibration.valid = ratio > 0.0f;

    calibrated = movementCalibration.valid;
    errorState = !movementCalibration.valid;

    encoder.resetPosition();
    motorStepCounter = 0;

    std::cout
        << "[CALIBRATION] Motor steps full range of motion: "
        << motorStepsBetweenLimits
        << " | Total Encoder: "
        << totalEncoderSteps
        << " | Ratio: "
        << ratio
        << std::endl;


    /* MAKE SURE THE DOOR IS LEFT CLOSED AFTER CALIBRATION */
    if (gpio_get(PIN_LMSW_MTR_OPEN)) runMotor(encoder, std::nullopt);
}



void Motor::recalibrationOnTouch()
{
    // Reset position when the open limit switch is reached
    if (gpio_get(PIN_LMSW_MTR_OPEN))
    {
        currentPositionSteps = 0;
        positionKnown = true;
    }
    // Set position to maximum when the closed limit switch is reached
    else if (motorStepsBetweenLimits > 0 &&
             gpio_get(PIN_LMSW_ENC_CLOSED))
    {
        currentPositionSteps = motorStepsBetweenLimits;
        positionKnown = true;
    }
}
/* MOTOR SETTERS */
void Motor::setMtrStpsPerEncder(float steps)
{
    this->movementCalibration.motorStepsPerEncoder = steps;
}

void Motor::setDirection(bool direction)
{
    this->direction = direction;
}

void Motor::setCalibState(const bool& calibState)
{
    this->calibrated = calibState;
}

void Motor::setMotorStepsBetweenLimits(int steps)
{
    motorStepsBetweenLimits = steps > 0 ? steps : 0;

    if (motorStepsBetweenLimits == 0)
    {
        currentPositionSteps = 0;
        positionKnown = false;
    }
}

/* MOTOR GETTERS */
float Motor::getMtrStpsPerEncder() const
{
    return this->movementCalibration.motorStepsPerEncoder;
}

bool Motor::getDirection() const
{
    return direction;
}

bool Motor::getErrorState() const
{
    return errorState;
}

int Motor::getMotorStepsBetweenLimits() const
{
    return motorStepsBetweenLimits;
}

/* ########## P R I V A T E ########## */


/* MOTOR COIL SEQUENCE
 * Each row represents one motor step
 * Each column represents one motor coil */
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


/* DISABLE MOTOR COILS */
void Motor::stopMotor()
{
    for (const auto& coil : motorCoils)
    {
        coil->write(false);
    }
}

/* REVERSE MOTOR MOVEMENT DIRECTION */
void Motor::reverseDirection()
{
    direction = !direction;
}

/* CHECK WHETHER STUCK DETECTION CAN BE USED */
bool Motor::stuckDetectionEnabled() const
{
    return calibrated && movementCalibration.motorStepsPerEncoder > 0.0f;
}

/* CALCULATE MOTOR STEPS REQUIRED FOR ONE ENCODER STEP */
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

void Motor::runMotorSteps()
{
    /* APPLY CURRENT COIL SEQUENCE */
    for (uint i = 0; i < MTR_COILS; ++i)
    {
        gpio_put(
            motorCoils[i]->getPin(),
            InstructionsMatrix[step][i]);
    }

    /* MOVE TO NEXT MOTOR STEP */
    if (direction)
    {
        step = step > 0 ? step - 1 : MTR_STEPS - 1;
    }
    else
    {
        step = step < MTR_STEPS - 1 ? step + 1 : 0;
    }
}

void Motor::checkForStuck()
{
    const int expectedMotorSteps = static_cast<int>(std::ceil(movementCalibration.motorStepsPerEncoder));

    /* ADD ALLOWED MOVEMENT TOLERANCE = 200
     * ALMOST ONE ENCODER POSITION SO IT WILL TRIGGER AS FAST AS POSSIBLE
     * EXAMPLE
     * Ratio: 205.827+200=406
     * Steps since encoder: 407 | Allowed: 406 */
    const int allowedMotorSteps = expectedMotorSteps + STUCK_STEP_TOLERANCE;

    if (stepsSinceEncoder > allowedMotorSteps)
    {
        errorState = true;
    }
}

// Check if the next motor step would exceed the travel limits
bool Motor::checkPositionLimit(bool directionAtStart) const
{
    // Only check travel limits when the current position is known
    if (positionKnown && motorStepsBetweenLimits > 0)
    {
        // Calculate the position after the next step
        const int predictedPosition =
            currentPositionSteps + (directionAtStart ? 1 : -1);

        // Stop if the next step would exceed the travel limits
        if (predictedPosition < 0 ||
            predictedPosition > motorStepsBetweenLimits)
        {
            return true;
        }
    }

    return false;
}

// Update motor position after movement finishes
void Motor::finalizeMotorPosition(bool directionAtStart, bool lmswMtrStop, bool lmswEncStop)
{
    // Mark position as unknown if a motor error occurred
    if (errorState)
    {
        positionKnown = false;
    }
    // Confirm OPEN endpoint
    else if (!directionAtStart &&
             (lmswMtrStop || gpio_get(PIN_LMSW_MTR_OPEN)))
    {
        currentPositionSteps = 0;
        positionKnown = true;
    }
    // Confirm CLOSED endpoint
    else if (directionAtStart &&
             motorStepsBetweenLimits > 0 &&
             (lmswEncStop || gpio_get(PIN_LMSW_ENC_CLOSED)))
    {
        currentPositionSteps = motorStepsBetweenLimits;
        positionKnown = true;
    }
}

//restores position on boot
void Motor::restorePosition(int steps, bool known)
{
    if (!known ||
        motorStepsBetweenLimits <= 0 ||
        steps < 0 ||
        steps > motorStepsBetweenLimits)
    {
        currentPositionSteps = 0;
        positionKnown = false;
        return;
    }

    currentPositionSteps = steps;
    positionKnown = true;

}