#include "wifi.h"
#include "../../macros/macros.h"
#include "../personal_config.h"
#include "pico/cyw43_arch.h"



WIFI::WIFI() : ssid(WIFI_SSID), password(WIFI_PASSWORD) {}

bool WIFI::init() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_FINLAND)) {
        return false;
    }
    return true;
}

bool WIFI::connectWifi() {

    // Make sure the LED is off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0) ;

    // Put the device into station mode
    cyw43_arch_enable_sta_mode();

    // Print a status message
    printf("Attempting connection . . . \n") ;

    // Connect to the network
    if (cyw43_arch_wifi_connect_timeout_ms(this->ssid, this->password, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECTING_TIMEOUT_MS)) {
       return false;
    }

    // Use the LED to indicate connection success
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1) ;

    // Report the IP, netmask, and gateway that we've been assigned
    printf("IP: %s\n", ip4addr_ntoa(netif_ip_addr4(netif_default))) ;
    printf("Mask: %s\n", ip4addr_ntoa(netif_ip_netmask4(netif_default))) ;
    printf("Gateway: %s\n", ip4addr_ntoa(netif_ip_gw4(netif_default))) ;

    return true ;

}