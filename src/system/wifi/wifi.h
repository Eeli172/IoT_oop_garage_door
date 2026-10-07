#ifndef IOT_OOP_GARAGE_DOOR_WIFI_H
#define IOT_OOP_GARAGE_DOOR_WIFI_H


class WIFI {
public:

    // get credentials from --> personal_config.h <-- (added into .gitignore)
    WIFI();

    // Init wireless chip
    bool init();

    // Try to connect wifi if there's no active connection
    bool connectWifi();

private:
    const char *ssid;
    const char *password;
};

#endif //IOT_OOP_GARAGE_DOOR_WIFI_H