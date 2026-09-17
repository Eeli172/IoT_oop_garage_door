#ifndef IOT_OOP_GARAGE_DOOR_GPIOS_H
#define IOT_OOP_GARAGE_DOOR_GPIOS_H

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


#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H