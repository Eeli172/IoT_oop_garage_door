#include "motor_run.h"
#include <iostream>
#include <memory>

#include "../interrupts/interrupts.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"

Motor::Motor(const std::array<int, TOTAL_COILS> &coilGpios, bool inverted) :
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
    for (const int &coil : coilGpios)
    {
        auto ptr = std::make_unique<CommonGPIO>(coil, false, false, inverted);
        this->motorCoils.push_back(std::move(ptr));
    }
}

void Motor::calibrateMotor() const
{
    bool limitFound = false;
    bool calibrated = false;

    while (!calibrated)
    {
        while (!queue_try_remove(&lmswEventQ, &limitFound))
        {
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < this->motorCoils.size(); j++)
                {
                    gpio_put(this->motorCoils[j]->getPin(), this->runInstructionsMatrix[i][j]);
                }
                sleep_ms(5);
            }
        }

        sleep_ms(100);
        while (!queue_try_remove(&lmswEventQ, &limitFound))
        {
            for (int i = 7; i >= 0; i--)
            {
                for (int j = static_cast<int>(this->motorCoils.size()); j >= 0; j--)
                {
                    gpio_put(this->motorCoils[j]->getPin(), this->runInstructionsMatrix[i][j]);
                }
                sleep_ms(5);
            }
        }

        calibrated = true;
    }
}


