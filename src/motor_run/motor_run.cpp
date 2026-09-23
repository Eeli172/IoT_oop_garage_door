#include "motor_run.h"
#include <memory>

#include "../interrupts/interrupts.h"
#include "../macros/macros.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"



Motor::Motor(const std::vector<int> &coilGpios, bool inverted) :
currentDoorState(CLOSED),
stepsToEnd(0),
step(0)
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
    
    gpio_set_irq_enabled(PIN_LMSW_MTR, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(PIN_LMSW_ENC, GPIO_IRQ_EDGE_FALL, true);
    
    while ( ( (direction && !lmswEncStop) || (!direction && !lmswMtrStop) ) && !sw1Stop)
    {        
        for (uint i = 0; i < MTR_COILS; i++)
        {
            gpio_put(motorCoils[i]->getPin(), runInstructionsMatrix[step][i]);
        }
        
        sleep_ms(1);
        updateStep(direction);

        queue_try_remove(&lmswMtrEventQ, &lmswMtrStop);
        queue_try_remove(&lmswEncEventQ, &lmswEncStop);
        queue_try_remove(&sw1EventQ, &sw1Stop);
    }
    
    this->direction = !this->direction;

    return sw1Stop;
}

void Motor::calibrateMotor()
{
    direction = true;
    if (!gpio_get(PIN_LMSW_ENC)) // because of this motor doesn't crash into the already pressed switch on start
    {
        runMotor();
    }
    else direction = !direction;

    sleep_ms(100);
    runMotor();
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
