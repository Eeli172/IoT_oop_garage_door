#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H

#include <memory>
#include <vector>

#include "../gpios/gpios.h"
#include "../gpios/encoder/encoder.h"
#include "../types/types.h"

class StatusLeds;
class System;

class Motor : public std::enable_shared_from_this<Motor>
{
public:
    struct MovementCalibration
    {
        float motorStepsPerEncoder;
        bool valid;
    };

    Motor(const std::vector<int>& coilGpios, 
        bool inverted
    );
    ~Motor() = default;

    Motor(const Motor&) = delete;
    Motor& operator=(const Motor&) = delete;


    void runMotorSteps();
    bool runMotor(Encoder& encoder);
    void stopMotor();

    void reverseDirection();
    bool reverseOnPressedLimitSwitch();

    bool getDirection() const;


    void calibrateMotor(Encoder& encoder);

    const MovementCalibration& getCalibration() const;


    bool isDoorStuck() const;
    void clearDoorStuck();

    bool getErrorState() const;


    void createStatusLeds();
    void updateStatusLeds();


    doorState_t getCurrentDoorState() const;
    doorState_t getPreviousDoorState() const;

    void setCurrentDoorState(const doorState_t& state);

private:

    static int InstructionsMatrix[8][4];

    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::shared_ptr<StatusLeds> statusLeds;

    doorState_t currentDoorState;
    doorState_t previousDoorState;

    int stepsToEnd;
    int step;
    int motorStepCounter;
    bool direction;
    bool errorState;
    bool calibrated;
    bool doorStuck;
    int stepsSinceEncoder;


    MovementCalibration movementCalibration;


    float calculateMotorStepsPerEncoder(
        int motorSteps,
        int encoderSteps) const;

    bool stuckDetectionEnabled() const;
    void checkForStuck();
    void handleDoorStuck(Encoder& encoder);
};

#endif // IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H