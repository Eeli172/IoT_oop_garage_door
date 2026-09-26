#ifndef IOT_OOP_GARAGE_DOOR_WIFI_H
#define IOT_OOP_GARAGE_DOOR_WIFI_H


class WIFI
{
public:
    WIFI();
    bool init();
    bool connectWifi();

private:
    const char *ssid;
    const char *password;
};

#endif //IOT_OOP_GARAGE_DOOR_WIFI_H