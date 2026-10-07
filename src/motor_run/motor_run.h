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

    /* 
    Run motor until stopped by:
        1. sw1 
        2. limit switches
        3. stuck detection
        4. mqtt command
    Return: true (stopped manually (sw1 || mqtt command))  ||  false (otherwise)
    */
    bool runMotor(Encoder& encoder, const std::optional<std::function<bool(const std::string &)>>& MqttStop);

    /*
    Calibrate motor, door is left closed after calibration.
    */
    void calibrateMotor(Encoder& encoder);

    // SETTERS
    void setMtrStpsPerEncder(float steps);
    void setDirection(bool direction);
    void setCalibState(const bool& calibState);
    
    // GETTERS
    float getMtrStpsPerEncder() const;
    bool getDirection() const;
    bool getErrorState() const;



private:
    // CLASS POINTERS
    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::unique_ptr<Switches> limitSwitches;

    // ATTRIBUTES
    static int InstructionsMatrix[8][4];
    int step;
    int motorStepCounter;
    bool direction; // true == moving towards the encoder
    bool errorState; // true == door stuck
    bool calibrated;
    int stepsSinceEncoder;
    MovementCalibration_t movementCalibration;


    // turn off the output of the motor coils
    void stopMotor();

    // direction = !direction
    void reverseDirection();

    bool stuckDetectionEnabled() const;
    
    float calculateMotorStepsPerEncoder(
        int motorSteps,
        int encoderSteps) const;

    void runMotorSteps();
    
    void checkForStuck();

};

#endif // IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H