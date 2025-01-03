/*
 * wifi.h
 *
 * Inspired by WifiHelper.h by jondurrant
 */

#ifndef SRC_WIFIHELPER_H_
#define SRC_WIFIHELPER_H_

#include <stdlib.h>
#include "pico/stdlib.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "pico/cyw43_arch.h"


#define CYW43_INIT_COMPLETE_BIT   0x1
#define WIFI_INIT_COMPLETE_BIT    0x2


class WifiConnection {
    public:
        void init();

        static void connect_task(void *params);

        bool get_ip_address(uint8_t *ip);
        char *ip_to_string(uint8_t *ip, char *ips);
        bool get_gateway_address(uint8_t *ip);
        bool get_net_mask(uint8_t *ip) ;
        bool get_mac_address(uint8_t *mac);
        bool get_mac_address_str(char *macStr);
        bool join();
        bool is_joined();
        void set_wifi_connect_retries(int retries) { wifi_connect_retries = retries;}
        int get_wifi_connect_retries() { return wifi_connect_retries; }
        void set_wifi_auth(int auth) { wifi_auth = auth; }
        int get_wifi_auth() { return wifi_auth; }
        void set_wifi_connect_timeout(int timeout) { wifi_connect_timeout = timeout; }
        int get_wifi_connect_timeout() { return wifi_connect_timeout; }
        void set_ssid(const char *ssid) { this->ssid = (char *)ssid; };
        void set_password(const char *password) { this->password = (char *)password; };
        char *get_ssid() { return ssid; };
        char *get_password() { return password; };

        bool wait_for_cyw43_init();
        bool wait_for_wifi_init();


        static WifiConnection& getInstance() {
            static WifiConnection instance;
            
            instance.init_event_group = NULL;
            instance.wifi_connect_retries = 3;
            instance.wifi_auth = CYW43_AUTH_WPA2_AES_PSK;
            instance.wifi_connect_timeout = 60000;

            return instance;
        }


    private:
        WifiConnection() {};

        EventGroupHandle_t init_event_group;
        TaskHandle_t wifi_task_handle;

        int wifi_connect_retries;
        int wifi_auth;
        int wifi_connect_timeout;
        char *ssid;
        char *password;

        void unblock_cyw43_init();
        void unblock_wifi_init();
        void block_wifi_init();
};

#endif

