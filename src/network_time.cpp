
#include <stdlib.h>
#include <string.h>
#include "network_time.h"
#include "pico/util/datetime.h"
#include "hardware/rtc.h"
#include "lwip/apps/sntp.h"




/**
 * This function is declared in $PICO_SDK/lib/lwip/src/apps/sntp/sntp.c, but not implemented
 * anywhere in that library code. It is called from the lwIP stack when the SNTP client gets 
 * a result back from an NTP server, and it's our responsibility to do something with that time. 
 * The NetworkTime class, (which is involved because it holds time zone offset state) provides 
 * the set_time_in_seconds() method to implement the RTC interface.
 */
void sntpSetTimeSec(uint32_t sec) {
    NetworkTime::getInstance().set_time_in_seconds(sec);
}


void NetworkTime::init() {
    xTaskCreate(time_task, "SNTP Task", 1024, this, 1, &time_task_handle);
}


/***
 * Set timezone offset
 * 
 * @param offsetHours - hours of offset -23 to + 23
 * @param offsetMinutes - for timezones that use odd mintes you can add or sub additional minutes
 */
void NetworkTime::sntp_set_timezone(int offsetHours, int offsetMinutes) {
    sntp_timezone_minutes_offset = (offsetHours * 60) + offsetMinutes;
}


/***
 * Add SNTP server - can call to add multiple servers
 * @param server - string name of server. Should remain in scope
 */
void NetworkTime::sntp_add_server(const char *server){
    sntp_setservername(sntp_server_count++, server);
}


/***
 * Start syncing Pico time with SNTP
 */
void NetworkTime::sntp_start_sync() {
    rtc_init();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();
}


/***
 * Actually sets the Pico real-time clock (RTC) with a specific time. In the case of this
 * class, that time should be coming from an NTP server. See the sntpSetTimeSec() function
 * (declared in this file but not a part of the NetworkTime class) for how this call is made.
 */
void NetworkTime::set_time_in_seconds(uint32_t sec) {
    datetime_t date;
    struct tm *timeinfo;

    time_t t = sec + (60 * sntp_timezone_minutes_offset);

    timeinfo = gmtime(&t);

    memset(&date, 0, sizeof(date));
    date.sec = timeinfo->tm_sec;
    date.min = timeinfo->tm_min;
    date.hour = timeinfo->tm_hour;
    date.day = timeinfo->tm_mday;
    date.month = timeinfo->tm_mon + 1;
    date.year = timeinfo->tm_year + 1900;

    rtc_set_datetime(&date);
}


void NetworkTime::time_task(void *params) {
    datetime_t d;

    printf("NTP TASK STARTED\n");
    NetworkTime *time = (NetworkTime *)params;
    WifiConnection *wifi = time->wifi;

    printf("NTP TASK WAITING FOR WIFI INIT\n");
    wifi->wait_for_wifi_init();
    printf("NTP TASK WIFI INIT COMPLETE\n");

    time->sntp_start_sync();

    for(;;) {
        if(rtc_get_datetime(&d)) {
            printf("RTC: %04d-%02d-%02d %02d:%02d:%02d\n",
                   d.year,
                   d.month,
                   d.day,
                   d.hour,
                   d.min,
                   d.sec);
        }

        vTaskDelay(5000);
    }
}
