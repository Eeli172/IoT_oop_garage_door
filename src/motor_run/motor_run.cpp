#include "motor_run.h"
#include <iostream>
#include <memory>
#include <any>

#include "../interrupts/interrupts.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"



Motor::Motor(const std::vector<int> &coilGpios, bool inverted) :
doorDirection(CLOSED),
stepsToEnd(0),
runInstructionsMatrix {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
}
{
    for (const auto &coilPin : coilGpios)
    {
        this->motorCoils.push_back(std::make_unique<CommonGPIO>(coilPin, false, false, inverted));
    }
}

void Motor::runMotor(const bool reverse)
{
    bool limitFound = false;
    bool manualStop = false;

    if (reverse) // || !queue_try_remove(&sw1EventQ, &manualStop
    {
        // TO THE LEFT (OPEN)
        while (!queue_try_remove(&lmswEventQ1, &limitFound) && !queue_try_remove(&sw1EventQ, &manualStop))
        {
            for (int i = 0; i < 8; i++)
            {
                for (size_t j = 0; j < this->motorCoils.size(); j++)
                {
                    gpio_put(this->motorCoils[j]->getPin(), this->runInstructionsMatrix[i][j]);
                }
                sleep_ms(1);
            }
        }
    }

    else
    {
        // TO THE RIGHT (CLOSED)
        while (!queue_try_remove(&lmswEventQ2, &limitFound) && !queue_try_remove(&sw1EventQ, &manualStop))
        {
            for (int i = 7; i >= 0; i--)
            {
                for (int j = static_cast<int>(this->motorCoils.size() - 1); j >= 0; j--)
                {
                    gpio_put(this->motorCoils[j]->getPin(), this->runInstructionsMatrix[i][j]);
                }
                sleep_ms(1);
            }
        }
    }

    if (limitFound && reverse)
    {
        this->doorDirection = CLOSED;
    }

    if (limitFound && !reverse)
    {
        this->doorDirection = OPEN;
    }
}

void Motor::calibrateMotor()
{
    if (!gpio_get(LMSW_2)) //because of this motor doesn't crash into the already pressed switch on start
    {
        runMotor(false);
    }

    sleep_ms(100);

    if (!gpio_get(LMSW_1)) //because of this motor doesn't crash into the already pressed switch on start
    {
        runMotor(true);
    }
}

doorDirection_t Motor::getDirection() const
{
    return this->doorDirection;
}

void Motor::setCurrentDirection(const doorDirection_t &dir)
{
    this->doorDirection = dir;
}


