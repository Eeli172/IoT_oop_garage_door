#ifndef IOT_OOP_GARAGE_DOOR_GPIOS_H
#define IOT_OOP_GARAGE_DOOR_GPIOS_H

#include <vector>
#include <cstdint>
#include <memory>

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



// SWITCH HUB (SW0, SW1, SW2)
class Switches {
public:
    Switches(const std::vector<int> &pins, bool input, bool pullup, bool inverted);
    Switches(const Switches &) = delete;
    ~Switches() = default;

    void initInterrupts(const std::vector<int> &swPins, const std::vector<bool> &initStates);
    void enableInterrupts(const std::vector<int> &swPins) const;

private:
    std::vector<std::unique_ptr<CommonGPIO>> switches;
};



// LED HUB (GPIO: 20, 21, 22)
class StatusLeds
{
public:
    StatusLeds(const std::vector<int> &ledGpios, bool inverted);
    StatusLeds(const StatusLeds &) = delete;
    ~StatusLeds() = default;

    void writeLed (int ledGpio) const;

private:
    std::vector<std::unique_ptr<CommonGPIO>> ledGpios;
};



#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H