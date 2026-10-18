#ifndef IOT_OOP_GARAGE_DOOR_LEDS_H
#define IOT_OOP_GARAGE_DOOR_LEDS_H

/* STL */
#include <memory>
#include <vector>

/* CUSTOM HEADERS */
#include <pico/types.h>
#include "../gpios.h"
#include "../../types/types.h"

class System;


/*
 *
 * ========================================================================
 * STATUS LEDS CLASS
 * ========================================================================
 *
 */

class StatusLeds {
public:

    /**
     * @brief Initializes the status LEDs using the specified GPIO pins
     * and a weak reference to the System instance.
     * @param ledGpios Vector containing the GPIO pins assigned to the LEDs.
     * @param sys Weak pointer to the System instance used to retrieve
     * the current door and error states.
     */
    StatusLeds(const std::vector<int>& ledGpios, const std::weak_ptr<System>& sys);

    StatusLeds(const StatusLeds&) = delete;

    ~StatusLeds() = default;


    // ========================================================================
    // LED CONTROL
    // ========================================================================

    /**
     * @brief Updates the LED states according to the current door state
     * and error state retrieved from the System class.
     * Needs to be called continuously to keep the LED states updated.
     */
    void updateCall();

    /**
     * @brief Enables or disables permission for updateCall() to turn
     * the LEDs on.
     * @param enbld true to allow the LEDs to be turned on, false otherwise.
     */
    void setEnabled(const bool& enbld);


private:

    // ========================================================================
    // LED OPERATIONS
    // ========================================================================

    /**
     * @brief Turns all status LEDs off.
     */
    void turnOff();

    /**
     * @brief Toggles the current ledValue after a timeout.
     */
    void toggleValue();

    /**
     * @brief Retrieves the current door state and error state
     * from the System class.
     */
    void getStates();

    /**
     * @brief Turns off all LEDs except the one corresponding to doorState.
     * Sets the remaining active LED according to the current ledValue.
     */
    void updateLeds();


    // ========================================================================
    // GPIO DEVICES AND SYSTEM REFERENCE
    // ========================================================================

    std::vector<std::unique_ptr<CommonGPIO>> ledGpios;
    std::weak_ptr<System> system;


    // ========================================================================
    // STATUS LED VARIABLES
    // ========================================================================

    doorState_t doorState;
    bool errorState;
    bool ledValue;
    bool enabled;
    absolute_time_t time_previous;
};


#endif // IOT_OOP_GARAGE_DOOR_LEDS_H