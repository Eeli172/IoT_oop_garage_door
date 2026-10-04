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

    void setInterrupt(const std::vector<int> &swPins, const bool& enable) const;

private:
    std::vector<std::unique_ptr<CommonGPIO>> switches;
};




#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H