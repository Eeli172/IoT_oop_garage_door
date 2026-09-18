#include "motor_run.h"
#include <iostream>
#include <memory>

Motor::Motor(const std::vector<int> &coilGpios, bool inverted) :
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

void Motor::calibrateMotor()
{

}


