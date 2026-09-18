#ifndef IOT_OOP_GARAGE_DOOR_GPIOS_H
#define IOT_OOP_GARAGE_DOOR_GPIOS_H

#include <vector>
#include <memory>

class CommonGPIO {
public:
    CommonGPIO(int pin, bool input, bool pullup, bool inverted);
    CommonGPIO(const CommonGPIO &) = delete;
    ~CommonGPIO();

    void write(bool value) const;
    bool read() const;
    explicit operator bool() const;

    static uint32_t pins_in_use;
    int pin;
    bool input;
    bool is_dormant;
};



class GPIOHub
{
public:
    virtual void setInterrupt(int gpio, bool interruptStatus) = 0;
    virtual void firstInitInterrupts() = 0;
    virtual ~GPIOHub() = default;
};



class Switches : public GPIOHub
{
public:
    Switches(int sw0Pin, int sw1Pin, int sw2Pin, bool input, bool pullup, bool inverted);
    Switches(const Switches &) = delete;
    ~Switches() override = default;
    void setInterrupt(int gpio, bool interruptStatus) override;
    void firstInitInterrupts() override;

private:
    CommonGPIO sw0;
    CommonGPIO sw1;
    CommonGPIO sw2;
};

class LimitSwitches : public GPIOHub
{
    public:
};


#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H