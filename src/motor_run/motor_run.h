#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
/* STL */
#include <memory>
#include <vector>

/* CUSTOM */
#include "../gpios/gpios.h"

typedef enum { CLOSED, IN_BETWEEN, OPEN } doorState_t;

class Motor
{
    public:
    Motor(const std::vector<int> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    bool runMotor();
    void calibrateMotor();
    doorState_t getCurrentState() const;
    doorState_t getPreviousState() const;
    void setCurrentState(const doorState_t &dir);
    void updateStep(bool opening);
    bool getDirection();

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;

    doorState_t currentDoorState;
    doorState_t previousDoorState;

    int stepsToEnd;
    static int runInstructionsMatrix[8][4];
    int step;
    bool direction; // true = opening, false = closing
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H    