#ifndef IOT_OOP_GARAGE_DOOR_WIFI_H
#define IOT_OOP_GARAGE_DOOR_WIFI_H


class WIFI {
public:

    /**
     * @brief Uses credentials from the personal_config.h file to initialize class attributes.
     */
    WIFI();

    // ========================================================================
    // WIFI HARDWARE
    // ========================================================================

    /**
     * @brief Activates cyw43 hardware drivers and architecture.
     * @return true if activation was successful, false otherwise.
     */
    bool init();

    /**
     * @brief Attempts connection to a Wi-Fi using the user credentials given in personal_config.h file.
     * @return true if connecting was successful and a Wi-Fi was found, false otherwise.
     */
    bool connectWifi();

private:
    const char *ssid;
    const char *password;
};

#endif //IOT_OOP_GARAGE_DOOR_WIFI_H