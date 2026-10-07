#ifndef IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
#define IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H

/* STL */
#include <memory>
#include <vector>
#include <functional>
#include <optional>

/* CUSTOM HEADERS*/
#include "../gpios/gpios.h"
#include "../gpios/encoder/encoder.h"
#include "../../types/types.h"

class Motor
{
public:
    /* CONSTRUCTOR / DESTRUCTOR */
    Motor(const std::vector<int>& coilGpios, bool inverted);
    ~Motor() = default;
    Motor(const Motor&) = delete;
    Motor& operator=(const Motor&) = delete;

    /*
    Run motor until stopped by:
        1. SW1
        2. Limit switches
        3. Stuck detection
        4. MQTT command

    Return:
        true  - stopped manually by SW1 or MQTT
        false - stopped by limit switch or error
    */
    bool runMotor(
        Encoder& encoder,
        const std::optional<std::function<bool(const std::string&)>>& MqttStop
    );

    /*Calibrate motor and leave the door in the closed position. */
    void calibrateMotor(Encoder& encoder);

    /* MOTOR SETTERS */
    void setMtrStpsPerEncder(float steps);
    void setDirection(bool direction);
    void setCalibState(const bool& calibState);


    /* MOTOR GETTERS */
    float getMtrStpsPerEncder() const;
    bool getDirection() const;
    bool getErrorState() const;

private:
    /* CLASS POINTERS */

    std::vector<std::unique_ptr<CommonGPIO>> motorCoils;
    std::unique_ptr<Switches> limitSwitches;


    /* MOTOR VARIABLES */

    static int InstructionsMatrix[8][4];
    int step;
    int motorStepCounter;
    bool direction; // true == moving towards the encoder
    bool errorState; // true == door stuck
    bool calibrated;
    int stepsSinceEncoder;
    MovementCalibration_t movementCalibration;


    /* MOTOR CONTROL */

    // Disable motor coil outputs
    void stopMotor();

    // Reverse the current motor direction
    void reverseDirection();

    // Check whether stuck detection is available
    bool stuckDetectionEnabled() const;

    // Calculate motor steps required for one encoder step
    float calculateMotorStepsPerEncoder(
        int motorSteps,
        int encoderSteps
    ) const;

    // Apply the current coil sequence and advance to the next step
    void runMotorSteps();

    // Check whether the motor has stopped moving
    void checkForStuck();
};


#endif // IOT_OOP_GARAGE_DOOR_MOTOR_RUN_H
