#include <cstdio>
#include "pico/stdlib.h"
#include "secrets.h"
#include "wifi.h"
#include "network_time.h"
#include "apa102.h"

extern "C" {
    #include "pico_led.h"
    #include "FreeRTOSConfig.h"
    #include "FreeRTOS.h"
    #include "task.h"
    #include "queue.h"
    #include "timers.h"
    #include "hardware/rtc.h"
    #include "lwip/tcp.h"
    #include "lwip/err.h"
    #include "lwip/pbuf.h"
    #include "lwip/api.h"

    #include "pico/cyw43_arch.h"
}

#define TCP_PORT 4242
#define MAX_RETRIES 5
#define BUFFER_SIZE 1024


WifiConnection& wifi = WifiConnection::getInstance();
NetworkTime& network_time = NetworkTime::getInstance();

APA102 led_strip(50);


typedef struct {
    struct tcp_pcb *pcb;
    uint8_t buffer[BUFFER_SIZE];
    uint16_t buffer_len;
} tcp_server_t;

static void tcp_server_close(tcp_server_t *state);


static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *packet_buf, err_t err) {
    tcp_server_t *state = (tcp_server_t*)arg;
    
    printf("IN tcp_server_recv\n");

    if(!packet_buf) {
        tcp_server_close(state);
        return ERR_OK;
    }
    
    if(packet_buf->tot_len > 0) {
        printf("RX %4d BYTES\n", packet_buf->tot_len);

        // Copy the received data into our buffer
        uint16_t copy_len = MIN(packet_buf->tot_len, BUFFER_SIZE - state->buffer_len);
        printf("COPYING %d BYTES\n", copy_len);
        pbuf_copy_partial(packet_buf, state->buffer + state->buffer_len, copy_len, 0);
        state->buffer_len += copy_len;
        printf("BUFFER LEN %d BYTES\n", state->buffer_len);
    }

    // When we've received a full strip's worth of data (TCP may split this up over multiple transmissions)
    if(state->buffer_len == led_strip.get_strip_len() * 4) {
        printf("COMPLETE BUFFER; UPDATING STRIP\n");
        for(int n = 0; n < led_strip.get_strip_len(); n++) {
            uint8_t brightness, red, green, blue;
            uint8_t *led_config;
            led_config = &state->buffer[n * 4];
            brightness = led_config[0];
            red = led_config[1];
            green = led_config[2];
            blue = led_config[3];
            led_strip.set_led(n, red, green, blue, brightness);
        }
        led_strip.update_strip();

        // Clear the buffer after processing
        state->buffer_len = 0;
    }
    
    // Free the received pbuf
    pbuf_free(packet_buf);

    printf("DONE tcp_server_recv\n");
    
    return ERR_OK;
}


static void tcp_server_err(void *arg, err_t err) {
    tcp_server_t *state = (tcp_server_t*)arg;
    printf("IN tcp_server_err\n");
    if(err != ERR_ABRT) {
        tcp_server_close(state);
    }
}


static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    tcp_server_t *state = (tcp_server_t*)arg;
    
    printf("IN tcp_server_accept\n");
    if(err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }
    
    // Store PCB and set callbacks
    state->pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);
    
    return ERR_OK;
}


tcp_server_t* tcp_server_init(void) {
    tcp_server_t *state = (tcp_server_t *)calloc(1, sizeof(tcp_server_t));
    if(!state) {
        return NULL;
    }
    
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if(!pcb) {
        free(state);
        return NULL;
    }
    
    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if(err != ERR_OK) {
        free(state);
        return NULL;
    }
    
    state->pcb = tcp_listen_with_backlog(pcb, 1);
    if(!state->pcb) {
        free(state);
        return NULL;
    }
    
    tcp_arg(state->pcb, state);
    tcp_accept(state->pcb, tcp_server_accept);
    
    return state;
}


static void tcp_server_close(tcp_server_t *state) {
    printf("IN tcp_server_close() \n ");
    if(state->pcb != NULL) {
        tcp_arg(state->pcb, NULL);
        tcp_close(state->pcb);
        state->pcb = NULL;
    }
}


// void tcp_server_task(void *pvParameters) {
//     tcp_server_t *state = tcp_server_init();
//     if(!state) {
//         printf("Failed to initialize TCP server\n");
//         vTaskDelete(NULL);
//         return;
//     }
    
//     for(;;) {
//         // The TCP/IP stack will handle incoming connections and callbacks
//         // We just need to give other tasks a chance to run
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }


// static void vPointlessTask(void *pvParameters) {
//     uint32_t counter = 0;

//     for(;;) {
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

//         counter++;
//     }
// }


static TaskHandle_t led_task_handle;

void led_task(void *pvParameters) {
    WifiConnection *wifi = (WifiConnection *)pvParameters;

    printf("LED TASK STARTED, WAITING FOR WIFI\n");

    wifi->wait_for_wifi_init();

    printf("LED TASK WIFI IS UP\n");

    tcp_server_t *state = tcp_server_init();
    if(!state) {
        printf("FAILED TO INITIALIZE TCP SERVER\n");
        vTaskDelete(NULL);
    }

    for(;;) {
        vTaskDelay(1000);
    }
}



void launch() {

    wifi.set_ssid(WIFI_SSID);
    wifi.set_password(WIFI_PASSWORD);

    printf("STARTING CYW43/WIFI INITIALIZATION\n");
    wifi.init();

    printf("STARTING NTP SYNC\n");
    network_time.sntp_set_timezone(-7);
    network_time.set_wifi_connection(&wifi);
    network_time.init();

    xTaskCreate(led_task, "LED Data Task", 1024, &wifi, 1, &led_task_handle);

    vTaskStartScheduler();
}


int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("UP\n");
    sleep_ms(500);

    printf("LAUNCHING\n");
    launch();
}
