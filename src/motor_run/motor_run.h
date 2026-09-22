#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
/* STL */
#include <memory>
#include <vector>

/* CUSTOM */
#include "../gpios/gpios.h"

typedef enum { CLOSING, OPENING, CLOSED, OPEN } doorState_t;

class Motor
{
    public:
    Motor(const std::vector<int> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    void runMotor(bool reverse);
    void calibrateMotor();
    [[nodiscard]] doorState_t getState() const;
    void setCurrentState(const doorState_t &dir);

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    doorState_t doorState;
    int stepsToEnd;
    static int runInstructionsMatrix[8][4];
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H    