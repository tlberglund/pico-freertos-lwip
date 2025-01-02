/*
 * WifiUtils.cpp
 *
 *  Created on: 24 Nov 2021
 *      Author: jondurrant and Tim Berglund
 */

#include <time.h>
#include "wifi_utils.h"
#include "pico/cyw43_arch.h"
#include "pico/util/datetime.h"
#include "hardware/rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/apps/sntp.h"



WifiUtils::WifiUtils() {
    sntp_server_count = 0;
    sntp_timezone_minutes_offset = 0;
    init_event_group = NULL;
    wifi_connect_retries = 3;
    wifi_auth = CYW43_AUTH_WPA2_AES_PSK;
    wifi_connect_timeout = 60000;
}


/***
 * Initialize the CYW43 network controller and connect to the wireless network specified by
 * the SSID and password set in the class. If the connection fails, continue retrying periodically.
 * All of this work happens in the WifiUtils::connect_task() FreeRTOS task, which this function 
 * creates.
 */
void WifiUtils::init() {
    if(init_event_group == NULL) {
        init_event_group = xEventGroupCreate();
    }

    xTaskCreate(connect_task, "Wifi Task", 1024, this, 1, &wifi_task_handle);
}


void WifiUtils::connect_task(void *params) {
    TaskHandle_t task;
    uint8_t ip[4];
    char ip_str[20];

    WifiUtils *wifi_utils = (WifiUtils *)params;

    printf("WIFI CONNECT TASK STARTING\n");

    int res = cyw43_arch_init();
    if(res) {
        printf("CYW43 INIT FAILED, EXITING connect_task\n");
        vTaskDelete(NULL);
    }

    cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM);

    wifi_utils->unblock_cyw43_init();

    // CYW43 link status appears to be inaccurate before the first attempt to join the 
    // network, so join before we loop
    printf("JOINING '%s'\n", wifi_utils->get_ssid());
    if(wifi_utils->join()) {
        printf("JOINED '%s'\n", wifi_utils->get_ssid());
        wifi_utils->unblock_wifi_init();
    } 
    else {
        printf("FAILED TO JOIN '%s'\n", wifi_utils->get_ssid());
    }

    for(;;) {
        if(wifi_utils->is_joined()) {
            vTaskDelay(5000);
        }
        else {
            printf("NOT CONNECTED TO WIFI\n", wifi_utils->get_ssid());
            printf("JOINING '%s'\n", wifi_utils->get_ssid());
            if(wifi_utils->join()) {
                wifi_utils->unblock_wifi_init();

                printf("JOINED '%s'\n", wifi_utils->get_ssid());

                // WifiUtils::getMACAddressStr(buffer);
                // printf("MAC ADDRESS: %s\n", buffer);

                wifi_utils->get_ip_address(ip);
                wifi_utils->ip_to_string(ip, ip_str);
                printf("IP ADDRESS: %s\n", ip_str);
            }
            else {
                printf("FAILED TO JOIN '%s'\n", wifi_utils->get_ssid());
            }
        }
    }
}


bool WifiUtils::join() {
    block_wifi_init();

    cyw43_arch_enable_sta_mode();

    printf("CONNECTING TO NETWORK '%s'\n", get_ssid());

    int r = -1;
    int attempts = 0;
    while(r < 0) {
        attempts++;
        r = cyw43_arch_wifi_connect_timeout_ms(get_ssid(), 
                                               get_password(),
                                               get_wifi_auth(),
                                               get_wifi_connect_timeout());

        if(r) {
            printf("FAILED TO JOIN NETWORK\n");
            if(attempts >= get_wifi_connect_retries()) {
                return false;
            }
            vTaskDelay(2000);
        }
    }

    unblock_wifi_init();
    return true;
}



/***
 * Get IP address of unit
 * @param ip - output uint8_t[4]
 */
bool WifiUtils::get_ip_address(uint8_t *ip) {
    if(is_joined()) {
        memcpy(ip, netif_ip4_addr(&cyw43_state.netif[0]), 4);
        return true;
    }
    else {
        return false;
    }
}


char *WifiUtils::ip_to_string(uint8_t *ip, char *ips) {
    strcpy(ips, ipaddr_ntoa(netif_ip4_addr(&cyw43_state.netif[0])));
    return ips;
}


/***
 * Get Gateway address
 * @param ip - output uint8_t[4]
 */
bool WifiUtils::get_gateway_address(uint8_t *ip) {
    if(is_joined()) {
        memcpy(ip, netif_ip4_gw(&cyw43_state.netif[0]), 4);
        return true;
    }
    else {
        return false;
    }
}



/***
 * Get Net Mask address
 * @param ip - output uint8_t[4]
 */
bool WifiUtils::get_net_mask(uint8_t *ip) {
    if(is_joined()) {
        memcpy(ip, netif_ip4_netmask(&cyw43_state.netif[0]), 4);
        return true;
    }
    else {
        return false;
    }
}


bool WifiUtils::get_mac_address(uint8_t *mac) {
    if(is_joined()) {
        memcpy(mac, cyw43_state.mac, 6);
        // cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA,  mac);
        return true;
    }
    else {
        return false;
    }
}


/***
 * Get the mac address as a string
 * @param macStr: pointer to string of at least 14 characters
 * @return true if successful
 */
bool WifiUtils::get_mac_address_str(char *macStr) {
    uint8_t mac[6];
    if(get_mac_address(mac)) {
        for(uint8_t i = 0; i < 6; i++) {
            if(mac[i] < 16) {
                sprintf(&macStr[i*2], "0%X", mac[i]);
            } else {
                sprintf(&macStr[i*2], "%X", mac[i]);
            }
        }
        macStr[13] = 0;
        return true;
    }
    return false;
}


/***
 * Returns if joined to the network and we have a link
 * @return true if joined.
 */
bool WifiUtils::is_joined() {
    if(cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) > 0) {
        unblock_wifi_init();
        return true;
    }
    else {
        block_wifi_init();
        return false;
    }
}


/***
 * Set timezone offset
 * @param offsetHours - hours of offset -23 to + 23
 * @param offsetMinutes - for timezones that use odd mintes you can add or sub additional minutes
 */
void WifiUtils::sntp_set_timezone(int offsetHours, int offsetMinutes) {
    sntp_timezone_minutes_offset = (offsetHours * 60) + offsetMinutes;
}

/***
 * Add SNTP server - can call to add multiple servers
 * @param server - string name of server. Should remain in scope
 */
void WifiUtils::sntp_add_server(const char *server){
    sntp_setservername(sntp_server_count++, server);
}

/***
 * Start syncing Pico time with SNTP
 */
void WifiUtils::sntp_start_sync() {
    rtc_init();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();
}

/***
 * Call back function used to set the RTC with the SNTP response
 * @param sec
 */
void WifiUtils::set_time_in_seconds(uint32_t sec) {
    datetime_t date;
    struct tm * timeinfo;

    time_t t = sec + (60 * sntp_timezone_minutes_offset);

    timeinfo = gmtime(&t);

    memset(&date, 0, sizeof(date));
    date.sec = timeinfo->tm_sec;
    date.min = timeinfo->tm_min;
    date.hour = timeinfo->tm_hour;
    date.day = timeinfo->tm_mday;
    date.month = timeinfo->tm_mon + 1;
    date.year = timeinfo->tm_year + 1900;

    rtc_set_datetime (&date);
}



bool WifiUtils::wait_for_wifi_init() {
    EventBits_t bits = xEventGroupWaitBits(
        init_event_group,
        WIFI_INIT_COMPLETE_BIT,
        pdFALSE,              // Don't clear bits after waiting
        pdTRUE,               // Wait for all bits (just one in this case)
        portMAX_DELAY
    );

    return (bits & WIFI_INIT_COMPLETE_BIT);
}



bool WifiUtils::wait_for_cyw43_init() {
    EventBits_t bits = xEventGroupWaitBits(
        init_event_group,
        CYW43_INIT_COMPLETE_BIT,
        pdFALSE,              // Don't clear bits after waiting
        pdTRUE,               // Wait for all bits (just one in this case)
        portMAX_DELAY
    );

    return (bits & CYW43_INIT_COMPLETE_BIT);
}

