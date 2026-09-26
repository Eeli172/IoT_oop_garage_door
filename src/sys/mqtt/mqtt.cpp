/* STL */
#include <iostream>
#include <ostream>

/* CUSTOM */
#include "mqtt.h"
#include "../wifi/wifi.h"



/* CLASS MQTTManager */
MQTTManager::MQTTManager() :
wiFiManager(WIFI())
{}

bool MQTTManager::initWiFi()
{
    if (!this->wiFiEnabled)
    {
        if (this->wiFiManager.init())
        {
            std::cout << "[OK] --> Wi-Fi Hardware initialization successful." << std::endl;
        }

        else
        {
            std::cout << "[FAIL] --> Wi-Fi Hardware initialization failed." << std::endl;
            return false;
        }
    }

    if (this->wiFiManager.connectWifi())
    {
        std::cout << "[OK] --> Wi-Fi connection successful." << std::endl;
        return true;
    }

    std::cout << "[ERROR] --> Wi-Fi connection failed." << std::endl;
    return false;
}

bool MQTTManager::subsribeToTopic(char* topic)
{


}

bool MQTTManager::publishToTopic(char* topic)
{

}

