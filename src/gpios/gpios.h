#ifndef IOT_OOP_GARAGE_DOOR_GPIOS_H
#define IOT_OOP_GARAGE_DOOR_GPIOS_H

/* STL */
#include <vector>
#include <cstdint>
#include <memory>

/*
 *
 * =====================================================================================================================
 * COMMON GPIO CLASS
 * =====================================================================================================================
 *
 */

class CommonGPIO {
public:

    /**
     * @brief Configures a single pin with the given parameters.
     * @param pin GPIO pin.
     * @param input true for input, false for output.
     * @param pullup true for pullup, false otherwise.
     * @param inverted true to invert the values upon activation of the pin.
     */
    CommonGPIO(int pin, bool input, bool pullup, bool inverted);

    CommonGPIO(const CommonGPIO &) = delete;
    ~CommonGPIO();

    // ========================================================================
    // GPIO CONTROL
    // ========================================================================

    /**
     * @brief Changes the state of an GPIO.
     * @param value Indicates the value to which the GPIO is changed to.
     */
    void write(bool value) const;

    /**
     * Reads the current value of a GPIO.
     * @return true if value was successfully read, false otherwise (error in reading).
     */
    bool read() const;

    /**
     * @brief Returns the instance's pin.
     * @return pin - an integer
     */
    int getPin();

    /**
     * @brief Determines whether the pin is usable.
     * Static classwide bit-masking is used to enable only one instance for each pin.
     * @return true if pin is usable, false otherwise.
     */
    explicit operator bool() const;

private:
    static uint32_t pins_in_use;
    int pin;
    bool input;
    bool is_dormant;
};

/*
 *
 * =====================================================================================================================
 * SWITCH HUB (LIMIT SWITCHES, SW0, SW1, SW2)
 * =====================================================================================================================
 *
 */

class Switches {
public:

    /**
     * @brief Initializes each pin in the vector with the given parameters.
     * @param pins Vector of GPIO pins.
     * @param input true to configure the pins as inputs, false for outputs.
     * @param pullup true to configure pins as pullups, false otherwise.
     * @param inverted true to invert the values upon activation of each pin.
     */
    Switches(const std::vector<int> &pins, bool input, bool pullup, bool inverted);

    Switches(const Switches &) = delete;
    ~Switches() = default;

    // ========================================================================
    // SWITCH HUB CONTROL
    // ========================================================================

    /**
     * @brief Takes a vector of GPIOs and changes them to the given state with the param 'enable'
     * @param swPins Vector of gpio pins.
     * @param enable either true or false --> (true for activation of interrupt, false for deactivation of interrupt)
     */
    void setInterrupt(const std::vector<int> &swPins, const bool& enable) const;

private:
    std::vector<std::unique_ptr<CommonGPIO>> switches;
};

#endif //IOT_OOP_GARAGE_DOOR_GPIOS_H