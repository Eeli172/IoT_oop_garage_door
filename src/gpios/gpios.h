#ifndef IOT_OOP_GARAGE_DOOR_GPIOS_H
#define IOT_OOP_GARAGE_DOOR_GPIOS_H

#include <array>
#include <cstdint>
#include "../macros/macros.h"

class CommonGPIO {
public:
    CommonGPIO(int pin, bool input, bool pullup, bool inverted);
    CommonGPIO(const CommonGPIO &) = delete;
    ~CommonGPIO();

    void write(bool value) const;
    bool read() const;
    int getPin();
    explicit operator bool() const;

private:
    static uint32_t pins_in_use;
    int pin;
    bool input;
    bool is_dormant;
};


// ABSTRACT CLASS 'HUB'
class GPIOHub
{
public:
    virtual void setInterrupt(int gpio, bool interruptStatus) = 0;
    virtual void firstInitInterrupts() = 0;
    virtual ~GPIOHub() = default;
};


// SWITCH HUB (SW0, SW1, SW2)
class Switches : public GPIOHub
{
    public:
    Switches(int sw0Pin, int sw1Pin, int sw2Pin, bool input, bool pullup, bool inverted);
    Switches(const Switches &) = delete;
    ~Switches() override = default;

    void setInterrupt(int gpio, bool interruptStatus) override;
    void firstInitInterrupts() override;

    private:
    std::array<CommonGPIO, TOTAL_SWITCHES> switches;
};



// LIMIT SWITCH HUB (LMSW_1 & LMSW_2)
class LimitSwitches : public GPIOHub
{
    public:
    LimitSwitches(int lmswPin1, int lmswPin2, bool inverted);
    LimitSwitches(const LimitSwitches &) = delete;
    ~LimitSwitches() override = default;

    void setInterrupt(int gpio, bool interruptStatus) override;
    void firstInitInterrupts() override;

    private:
    std::array<CommonGPIO, TOTAL_LIMIT_SWITCHES> limitSwitches;
};



// LED HUB (GPIO: 20, 21, 22)



#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H