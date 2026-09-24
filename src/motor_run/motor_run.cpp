#include "motor_run.h"
#include <memory>

#include "../interrupts/interrupts.h"
#include "../macros/macros.h"
#include "../gpios/leds/leds.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"



Motor::Motor(const std::vector<int> &coilGpios, bool inverted) :
currentDoorState(CLOSED),
stepsToEnd(0),
step(0),
direction(true),
errorState(false), 
calibrated(false)
{
    for (const auto &coilPin : coilGpios)
    {
        this->motorCoils.push_back(std::make_unique<CommonGPIO>(coilPin, false, false, inverted));
    }
}

int Motor::runInstructionsMatrix[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

bool Motor::runMotor()
{
    bool lmswMtrStop = false;
    bool lmswEncStop = false;
    bool sw1Stop = false;

    while ( ( (direction && !lmswEncStop) || (!direction && !lmswMtrStop) ) && !sw1Stop)
    {        
        for (uint i = 0; i < MTR_COILS; i++)
        {
            gpio_put(motorCoils[i]->getPin(), runInstructionsMatrix[step][i]);
        }
        
        sleep_ms(1);
        updateStep(direction);

        queue_try_remove(&sw1EventQ, &sw1Stop);
        queue_try_remove(&lmswMtrEventQ, &lmswMtrStop);
        queue_try_remove(&lmswEncEventQ, &lmswEncStop);
    }
    
    this->direction = !this->direction;

    return sw1Stop;
}

void Motor::calibrateMotor()
{
    if (!gpio_get(PIN_LMSW_ENC)) // because of this motor doesn't crash into the already pressed switch on start
    {
        runMotor();
    }
    else direction = !direction;

    sleep_ms(100);
    runMotor();

    this->calibrated = true;
    this->errorState = false;
}

doorState_t Motor::getCurrentState() const
{
    return this->currentDoorState;
}

doorState_t Motor::getPreviousState() const
{
    return this->previousDoorState;
}

void Motor::setCurrentState(const doorState_t &dir)
{
    this->previousDoorState = this->currentDoorState;
    this->currentDoorState = dir;
    //this->updateStatusLeds();
}

void Motor::updateStep(bool opening) {
    if (opening) {
        if (step > 0) step--;
        else step = MTR_STEPS-1;
    }
    else {
        if (step < MTR_STEPS-1) step++;
        else step = 0;
    }
}

bool Motor::getDirection() {
    return direction;
}

bool Motor::getErrorState() {
    return this->errorState;
}

void Motor::createStatusLeds() {
    this->statusLeds = std::make_shared<StatusLeds>(
        std::vector{PIN_LED_1, PIN_LED_2, PIN_LED_3}, 
        weak_from_this()
    );
}

void Motor::updateStatusLeds() {
    this->statusLeds->updateCall();
}