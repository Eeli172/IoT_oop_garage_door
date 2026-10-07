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

Motor::Motor(
    const std::vector<int>& coilGpios,
    bool inverted) :
    step(0),
    motorStepCounter(0),
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
bool Motor::runMotor(Encoder& encoder, const std::optional<std::function<bool(const std::string &)>> &MqttStop)
{
    bool lmswMtrStop = false;
    bool lmswEncStop = false;
    bool manualStop = false;

    motorStepCounter = 0;
    stepsSinceEncoder = 0;
    errorState = false;

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

        runMotorSteps();

        sleep_us(MOTOR_STEP_DELAY_US);

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

        queue_try_remove(&lmswMtrEventQ, &lmswMtrStop);
        queue_try_remove(&lmswEncEventQ, &lmswEncStop);
    }

    limitSwitches->setInterrupt(std::vector{PIN_LMSW_MTR_OPEN, PIN_LMSW_ENC_CLOSED}, false);

    this->reverseDirection();

    if (lmswEncStop)
    {
        encoder.resetPosition();
        printf("ENC CLOSED: position reset to 0\n");
    }

    if (lmswMtrStop)
    {
        encoder.setPosition(65);
    }

    stopMotor();

    if (errorState)
    {
        return false;
    }

    return manualStop;
}


/*
Calibrate motor, door is left closed after calibration.
*/
void Motor::calibrateMotor(Encoder& encoder)
{
    this->direction = false;
    if (gpio_get(PIN_LMSW_MTR_OPEN)) this->direction = true;  
    
    calibrated = false;
    errorState = false;

    int totalMotorSteps = 0;
    int totalEncoderSteps = 0;

    bool reachedMtr(false);

    for (int run = 0; run < 2; ++run)
    {
        encoder.resetPosition();
        motorStepCounter = 0;

        runMotor(encoder, std::nullopt);

        totalMotorSteps += motorStepCounter;
        totalEncoderSteps += std::abs(encoder.getPosition());

        if (gpio_get(PIN_LMSW_MTR_OPEN))
        {
            reachedMtr = true;
        }

        // Only stop at MTR after ENC has been touched
        if (reachedMtr == true && gpio_get(PIN_LMSW_ENC_CLOSED))
        {
            break;
        }
    }


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
        << "[CALIBRATION] Total Motor: "
        << totalMotorSteps
        << " | Total Encoder: "
        << totalEncoderSteps
        << " | Ratio: "
        << ratio
        << std::endl;

    // make sure the door is left closed after calibration
    if (gpio_get(PIN_LMSW_MTR_OPEN)) runMotor(encoder, std::nullopt);
}




// SETTERS
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




// GETTERS
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




/* ########## P R I V A T E ########## */


// onko tarkoituksella constructorin ulkona? @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
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


// turn off the output of the motor coils
void Motor::stopMotor()
{
    for (const auto& coil : motorCoils)
    {
        coil->write(false);
    }
}

void Motor::reverseDirection()
{
    direction = !direction;
}

bool Motor::stuckDetectionEnabled() const
{
    return calibrated && movementCalibration.motorStepsPerEncoder > 0.0f;
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