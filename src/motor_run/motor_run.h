#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#include "../gpios/gpios.h"
#include <vector>

class Motor
{
    public:
    Motor(const std::array<int, TOTAL_COILS> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    void runMotor();
    void calibrateMotor() const;

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    int stepsToEnd;
    int runInstructionsMatrix[8][4];
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H