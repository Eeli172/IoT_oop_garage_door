#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#include <memory>
#include <vector>
#include <any>

#include "../gpios/gpios.h"

typedef enum { CLOSING, OPENING, CLOSED, OPEN } doorDirection_t;

class Motor
{
    public:
    Motor(const std::vector<int> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    void runMotor(bool reverse);
    void calibrateMotor();
    [[nodiscard]] doorDirection_t getDirection() const;
    void setCurrentDirection(const doorDirection_t &dir);

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    doorDirection_t doorDirection;
    int stepsToEnd;
    int runInstructionsMatrix[8][4];
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H    