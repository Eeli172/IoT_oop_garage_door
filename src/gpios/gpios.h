#ifndef IOT_OOP_GARAGE_DOOR_GPIOS_H
#define IOT_OOP_GARAGE_DOOR_GPIOS_H

#include <vector>
#include <array>
#include <cstdint>
#include <memory>
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



// SWITCH HUB (SW0, SW1, SW2)
class Switches {
public:
    Switches(std::vector<int> pins, bool input, bool pullup, bool inverted);
    Switches(const Switches &) = delete;
    ~Switches() = default;

    void firstInitInterrupts();

private:
    std::vector<std::unique_ptr<CommonGPIO>> switches;
};


// LED HUB (GPIO: 20, 21, 22)
class StatusLeds
{
public:
    StatusLeds(std::array<int, TOTAL_LEDS> &ledGpios, bool inverted);
    StatusLeds(const StatusLeds &) = delete;
    ~StatusLeds() = default;

    void writeLed (const bool &ledState, int ledGpio) const;

private:
    std::array<std::unique_ptr<CommonGPIO>, TOTAL_LEDS> ledGpios;
};



#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H