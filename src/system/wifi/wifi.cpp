#include "wifi.h"
#include "../../macros/macros.h"
#include "../personal_config.h"
#include "pico/cyw43_arch.h"

/* ########## P U B L I C ########## */

WIFI::WIFI() : ssid(WIFI_SSID), password(WIFI_PASSWORD) {}


bool WIFI::init() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_FINLAND)) {
        return false;
    }
    return true;
}

bool WIFI::connectWifi() {
    if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        cyw43_arch_enable_sta_mode();
        printf("Attempting connection . . . \n") ;

        if (cyw43_arch_wifi_connect_timeout_ms(
            this->ssid,
            this->password,
            CYW43_AUTH_WPA2_AES_PSK,
            WIFI_CONNECTING_TIMEOUT_MS
        )) { return false; }

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1) ;

        printf("IP: %s\n", ip4addr_ntoa(netif_ip_addr4(netif_default))) ;
        printf("Gateway: %s\n", ip4addr_ntoa(netif_ip_gw4(netif_default))) ;
    }

    return true ;
}




/* ########## P R I V A T E ########## */
