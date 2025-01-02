/*
 * WifiHelper.h
 *
 *  Created on: 24 Nov 2021
 *      Author: jondurrant
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


class WifiUtils {
public:
    WifiUtils();

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
    void sntp_set_timezone(int offsetHours, int offsetMinutes = 0);
    void sntp_add_server(const char *server);
    void sntp_start_sync();
    void set_time_in_seconds(uint32_t sec);
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
    void unblock_cyw43_init() { xEventGroupSetBits(init_event_group, CYW43_INIT_COMPLETE_BIT); };
    void unblock_wifi_init() { xEventGroupSetBits(init_event_group, WIFI_INIT_COMPLETE_BIT); };
    void block_wifi_init() { xEventGroupClearBits(init_event_group, WIFI_INIT_COMPLETE_BIT); };


private:
    EventGroupHandle_t init_event_group;
    TaskHandle_t wifi_task_handle;

    int sntp_server_count;
    int32_t sntp_timezone_minutes_offset;
    int wifi_connect_retries;
    int wifi_auth;
    int wifi_connect_timeout;
    char *ssid;
    char *password;
};

#endif

