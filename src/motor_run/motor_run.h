#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
/* STL */
#include <memory>
#include <vector>

/* CUSTOM */
#include "../gpios/gpios.h"
#include "../types/types.h"



class StatusLeds;

class Motor : public std::enable_shared_from_this<Motor>
{
    public:
    Motor(const std::vector<int> &coilGpios, bool inverted);
    Motor(const Motor &) = delete; // Copy-constructors denied.
    ~Motor() = default;

    bool runMotor();
    void calibrateMotor();
    void createStatusLeds();
    void updateStatusLeds();
    void setCurrentState(const doorState_t &dir);
    void updateStep(bool opening);
    doorState_t getCurrentState() const;
    doorState_t getPreviousState() const;
    bool getErrorState();
    bool getDirection();
    

    private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::shared_ptr<StatusLeds> statusLeds;

    doorState_t currentDoorState;
    doorState_t previousDoorState;

    int stepsToEnd;
    static int runInstructionsMatrix[8][4];
    int step;
    bool direction; // true = opening, false = closing
    bool errorState; // true = error = door stuck
    bool calibrated;
};


#endif //IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H    