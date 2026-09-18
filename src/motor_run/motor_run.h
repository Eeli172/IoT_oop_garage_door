#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#include "../gpios/gpios.h"

class Motor
{
    public:
    Motor(const std::vector<int> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    void runMotor();
    void calibrateMotor();
    void initializeInterrupts();

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    int runInstructionsMatrix[8][4];
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H