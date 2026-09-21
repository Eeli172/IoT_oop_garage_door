#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#include <memory>
#include <vector>

#include "../gpios/gpios.h"


class Motor
{
    public:
    Motor(const std::vector<int> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    void runMotor(bool reverse);
    void calibrateMotor() const;

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    int stepsToEnd;
    int runInstructionsMatrix[8][4];
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H    