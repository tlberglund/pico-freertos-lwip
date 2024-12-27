#include <cstdio>
#include "pico/stdlib.h"
#include "secrets.h"
#include "wifi_utils.h"

extern "C" {
    #include "pico_led.h"
    #include "FreeRTOSConfig.h"
    #include "FreeRTOS.h"
    #include "task.h"
    #include "queue.h"
    #include "timers.h"
    #include "hardware/rtc.h"
    // #include "lwip/tcp.h"
    // #include "lwip/err.h"
    // #include "lwip/pbuf.h"
    // #include "lwip/api.h"

    #include "pico/cyw43_arch.h"
}

#if 0
#define TCP_PORT 4242
#define MAX_RETRIES 5
#define BUFFER_SIZE 1024


typedef struct {
    struct tcp_pcb *pcb;
    uint8_t buffer[BUFFER_SIZE];
    uint16_t buffer_len;
} TCP_SERVER_T;

static void tcp_server_close(TCP_SERVER_T *state);


static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    
    printf("IN tcp_server_recv\n");

    if (!p) {
        // Remote host closed connection
        tcp_server_close(state);
        return ERR_OK;
    }
    
    if (p->tot_len > 0) {
        printf("RX LEN=%d\n", p->tot_len);

        // Copy the received data into our buffer
        uint16_t copy_len = MIN(p->tot_len, BUFFER_SIZE - state->buffer_len);
        pbuf_copy_partial(p, state->buffer + state->buffer_len, copy_len, 0);
        state->buffer_len += copy_len;
        
        // Process the received data here
        // For this example, we'll just echo it back
        tcp_write(pcb, state->buffer, state->buffer_len, TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        
        // Clear the buffer after processing
        state->buffer_len = 0;
    }
    
    // Free the received pbuf
    pbuf_free(p);

    printf("DONE tcp_server_recv\n");
    
    return ERR_OK;
}


static void tcp_server_err(void *arg, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("IN tcp_server_err\n");
    if (err != ERR_ABRT) {
        tcp_server_close(state);
    }
}


static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    
    printf("IN tcp_server_accept\n");
    if (err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }
    
    // Store PCB and set callbacks
    state->pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);
    
    return ERR_OK;
}


TCP_SERVER_T* tcp_server_init(void) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        return NULL;
    }
    
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        free(state);
        return NULL;
    }
    
    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err != ERR_OK) {
        free(state);
        return NULL;
    }
    
    state->pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->pcb) {
        free(state);
        return NULL;
    }
    
    tcp_arg(state->pcb, state);
    tcp_accept(state->pcb, tcp_server_accept);
    
    return state;
}


static void tcp_server_close(TCP_SERVER_T *state) {
    printf("IN tcp_server_close() \n ");
    if (state->pcb != NULL) {
        tcp_arg(state->pcb, NULL);
        tcp_close(state->pcb);
        state->pcb = NULL;
    }
}


void tcp_server_task(void *pvParameters) {
    TCP_SERVER_T *state = tcp_server_init();
    if (!state) {
        printf("Failed to initialize TCP server\n");
        vTaskDelete(NULL);
        return;
    }
    
    while (1) {
        // The TCP/IP stack will handle incoming connections and callbacks
        // We just need to give other tasks a chance to run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif


static void vPointlessTask(void *pvParameters) {
    uint32_t counter = 0;

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        counter++;
    }
}



void main_task(void *params){
	datetime_t d;

	printf("Main task started\n");

	if (WifiUtils::init()){
		printf("Wifi Controller Initialised\n");
	} else {
		printf("Failed to initialise controller\n");
		return;
	}


	printf("Connecting to WiFi... %s \n", WIFI_SSID);

	if (WifiUtils::join(WIFI_SSID, WIFI_PASSWORD)){
		printf("Connect to Wifi\n");
	} else {
		printf("Failed to connect to Wifi \n");
	}


	//Print MAC Address
	char macStr[20];
	WifiUtils::getMACAddressStr(macStr);
	printf("MAC ADDRESS: %s\n", macStr);

	//Print IP Address
	char ipStr[20];
	WifiUtils::getIPAddressStr(ipStr);
	printf("IP ADDRESS: %s\n", ipStr);

	WifiUtils::sntpAddServer("0.us.pool.ntp.org");
	WifiUtils::sntpAddServer("1.us.pool.ntp.org");
	WifiUtils::sntpAddServer("2.us.pool.ntp.org");
	WifiUtils::sntpAddServer("3.us.pool.ntp.org");
	WifiUtils::sntpSetTimezone(0);
	WifiUtils::sntpStartSync();

    while(true) {

    	//runTimeStats();

        if(rtc_get_datetime(&d)) {
            printf("RTC: %d-%d-%d %d:%d:%d\n",
                   d.year,
                   d.month,
                   d.day,
                   d.hour,
                   d.min,
                   d.sec);
        }

        vTaskDelay(3000);

        if (!WifiUtils::isJoined()){
        	printf("AP Link is down\n");

        	if (WifiUtils::join(WIFI_SSID, WIFI_PASSWORD)){
				printf("Connect to Wifi\n");
			} else {
				printf("Failed to connect to Wifi \n");
			}
        }
    }
}


void vLaunch( void) {
    TaskHandle_t task;

    xTaskCreate(main_task, "MainThread", 2048, NULL, 1, &task);

    vTaskStartScheduler();
}


int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("UP\n");

    sleep_ms(1000);

    printf("LAUNCHING\n");
    vLaunch();
}
