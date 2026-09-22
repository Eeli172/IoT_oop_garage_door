#include "motor_run.h"
#include <memory>

#include "../interrupts/interrupts.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"



Motor::Motor(const std::vector<int> &coilGpios, bool inverted) :
doorState(CLOSED),
stepsToEnd(0)
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

void Motor::runMotor(const bool reverse)
{
    uint limitGpio;
    bool manualStopGpio = false;

    if (reverse) // || !queue_try_remove(&sw1EventQ, &manualStop
    {
        // TO THE LEFT (OPEN)
        printf("A\n");

        while (!queue_try_remove(&lmswEventQ, &limitGpio) && !queue_try_remove(&sw1EventQ, &manualStopGpio))
        {
            printf("B\n");
            for (int i = 0; i < 8; i++)
            {
                for (size_t j = 0; j < this->motorCoils.size(); j++)
                {
                    gpio_put(this->motorCoils[j]->getPin(), this->runInstructionsMatrix[i][j]);
                }
                sleep_ms(1);
            }
        }

        if (!limitGpio) gpio_set_irq_enabled(limitGpio, GPIO_IRQ_EDGE_FALL, true);
        printf("C\n");
    }

    else
    {
        printf("D\n");
        // TO THE RIGHT (CLOSED)
        while (!queue_try_remove(&lmswEventQ, &limitGpio) && !queue_try_remove(&sw1EventQ, &manualStopGpio))
        {
            printf("E\n");
            for (int i = 7; i >= 0; i--)
            {
                for (int j = static_cast<int>(this->motorCoils.size() - 1); j >= 0; j--)
                {
                    gpio_put(this->motorCoils[j]->getPin(), this->runInstructionsMatrix[i][j]);
                }
                sleep_ms(1);
            }
        }

        if (!limitGpio) gpio_set_irq_enabled(limitGpio, GPIO_IRQ_EDGE_FALL, true);

        printf("F\n");
    }
}

void Motor::calibrateMotor()
{
    if (!gpio_get(LMSW_2)) //because of this motor doesn't crash into the already pressed switch on start
    {
        runMotor(false);
    }

    sleep_ms(100);

    runMotor(true);
}

doorState_t Motor::getState() const
{
    return this->doorState;
}

void Motor::setCurrentState(const doorState_t &dir)
{
    this->doorState = dir;
}


