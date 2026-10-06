#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H

/* STL */
#include <memory>
#include <vector>
#include <functional>
#include <optional>

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
    bool runMotor(Encoder& encoder, const std::optional<std::function<bool(const std::string &)>>& MqttStop);
    void stopMotor();

    void reverseDirection();
    bool reverseOnPressedLimitSwitch();

    bool getDirection() const;
    void setDirection(bool direction);

    float getMtrStpsPerEncder() const;
    void setMtrStpsPerEncder(float steps);

    void calibrateMotor(Encoder& encoder);
    void setCalibState(const bool& calibState);

    void clearErrorState();
    bool getErrorState() const;

private:
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::unique_ptr<Switches> limitSwitches;

    static int InstructionsMatrix[8][4];    
    int step;
    int motorStepCounter;
    bool direction;
    bool errorState;
    bool calibrated;
    int stepsSinceEncoder;


    MovementCalibration_t movementCalibration;


    float calculateMotorStepsPerEncoder(
        int motorSteps,
        int encoderSteps) const;

    bool stuckDetectionEnabled() const;
    void checkForStuck();
};

#endif // IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H