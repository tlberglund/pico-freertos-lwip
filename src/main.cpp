
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

extern "C" {
#define traceENTER_vTaskStartScheduler() printf("IN vTaskStartScheduler()")
#define traceRETURN_xTaskCreate(x) printf("traceRETURN_xTaskCreate RETURNING %d\n",x);
#define traceENTER_xTaskCreate( pxTaskCode, pcName, uxStackDepth, pvParameters, uxPriority, pxCreatedTask ) printf("traceENTER_xTaskCreate: pxTaskCode=%08x, pcName=%s, uxStackDepth=%d, pvParameters=%08x, uxPriority=%d, pxCreatedTask=%08x\n", pxTaskCode, pcName, uxStackDepth, pvParameters, uxPriority, pxCreatedTask);
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

// uint8_t ucHeap[configTOTAL_HEAP_SIZE] __attribute__((aligned(8)));

void SysTick_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);

}



int pico_led_init(void) {
    return cyw43_arch_init();
}

void pico_set_led(bool led_on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}



void blinkingTask(void *pvParameters)
{
for(;;) sleep_ms(10);
    pico_set_led(true);

    for(;;)
    {
        pico_set_led(true);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        pico_set_led(false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


char buf[5000];

int main() {
    stdio_init_all();

    pico_led_init();
    pico_set_led(true);
    sleep_ms(2000);
    pico_set_led(false);

    printf("FreeRTOS\n");


    BaseType_t xReturned;
    TaskHandle_t xHandle = (TaskHandle_t)0x1234;
    xReturned = xTaskCreate(
                    blinkingTask,
                    "LED task",
                    512,
                    ( void * ) 0xcafebabe,
                    5,
                    &xHandle );

    printf("xReturned=%d\n", xReturned);
#if 0
    printf("xHandle=%08x\n", xHandle);


    vTaskList(buf);
    printf("\r\nTask List:\r\n");
    printf("Name          State  Priority  Stack   Num\r\n");
    printf("*******************************************\r\n");
    printf("%s\r\n", buf);



    // Alternative: just get number of tasks
    UBaseType_t num_tasks = uxTaskGetNumberOfTasks();
    printf("Number of tasks: %u\n", num_tasks);

#endif
    printf("ABOUT TO START SCHEDULER\n");
    // vTaskStartScheduler();
    printf("SCHEDULER NOT STARTED\n");

    while(1) {
        sleep_ms(1000);
        printf("heartbeat\n");
    }

    while(1)
    {
        configASSERT(0);
    }
}


// void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
//     printf("Stack overflow in task: %s\n", pcTaskName);
//     while(1); // Halt for debugging
// }

// void vApplicationMallocFailedHook(void) {
//     printf("Malloc failed!\n");
//     while(1); // Halt for debugging
// }

// void vApplicationTickHook(void) {
//     pico_set_led(true);
//     // static uint32_t tick_count = 0;
//     // if(++tick_count % configTICK_RATE_HZ == 0) {  // Print every second
//     //     printf("Tick: %lu\n", tick_count);
//     // }
// }

// void vApplicationIdleHook(void) {
// }
