#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H

/* STL */
#include <memory>
#include <vector>

/* CUSTOM HEADERS */
#include "../gpios/gpios.h"
#include "../gpios/encoder/encoder.h"
#include "../../types/types.h"

class Motor {
public:
    Motor(const std::vector<int>& coilGpios, bool inverted);
    ~Motor() = default;
    Motor(const Motor&) = delete;
    Motor &operator=(const Motor&) = delete;

    void runMotorSteps();
    bool runMotor(Encoder& encoder);
    void stopMotor();

    void reverseDirection();
    bool reverseOnPressedLimitSwitch();

    bool getDirection() const;
    
    float getMtrStpsPerEncder() const;
    void setMtrStpsPerEncder(float steps);

    void calibrateMotor(Encoder& encoder);

    void clearErrorState();
    bool getErrorState() const;

private:

    static int InstructionsMatrix[8][4];

    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;

    int step;
    int motorStepCounter;
    bool direction;
    bool errorState;
    bool calibrated;
    int stepsSinceEncoder;


    MovementCalibration movementCalibration;


    float calculateMotorStepsPerEncoder(
        int motorSteps,
        int encoderSteps) const;

    bool stuckDetectionEnabled() const;
    void checkForStuck();
};

#endif // IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H