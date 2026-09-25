#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H

#include <memory>
#include <vector>

#include "../gpios/gpios.h"
#include "../types/types.h"
#include "../gpios/encoder/encoder.h"

class StatusLeds;

class Motor : public std::enable_shared_from_this<Motor>
{
public:
    struct MovementCalibration
    {
        int motorStepsOpening;
        int encoderStepsOpening;
        int motorStepsClosing;
        int encoderStepsClosing;
        bool valid;
    };

    Motor(const std::vector<int>& coilGpios, bool inverted);
    Motor(const Motor&) = delete;
    ~Motor() = default;

    bool runMotor(Encoder& encoder);
    void calibrateMotor(Encoder& encoder);

    void createStatusLeds();
    void updateStatusLeds();

    void setCurrentState(const doorState_t& dir);
    void updateStep(bool opening);

    doorState_t getCurrentState() const;
    doorState_t getPreviousState() const;

    bool getErrorState();
    bool getDirection();

    const MovementCalibration& getCalibration() const;
    bool isDoorStuck() const;

    void clearDoorStuck();
    void reverseDirection();

private:
    void stopMotor();
    void processEncoderEvents(Encoder& encoder);
    bool checkForStuck(Encoder& encoder);
    void resetStuckCheck(Encoder& encoder);
    void handleDoorStuck(Encoder& encoder);

    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::shared_ptr<StatusLeds> statusLeds;

    doorState_t currentDoorState;
    doorState_t previousDoorState;

    int stepsToEnd;
    static int runInstructionsMatrix[8][4];
    int step;

    bool direction;
    bool errorState;
    bool calibrated;

    int motorStepCounter;

    struct
    {
        int motorSteps;
        int encoderPosition;
    } stuckCheck;

    MovementCalibration movementCalibration;
    bool doorStuck;
};

#endif
