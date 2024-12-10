
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
extern "C" {
    #include "FreeRTOS.h"
    #include "task.h"
}

int pico_led_init(void) {
    return cyw43_arch_init();
}

void pico_set_led(bool led_on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}


void blinkingTask(void *pvParameters)
{
    pico_set_led(true);
    printf("TASK STARTING WITH %08x\n", pvParameters);

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

    printf("xReturned=%08x\n", xReturned);
    printf("xHandle=%08x\n", xHandle);


    TaskHandle_t task_handle = xTaskGetHandle("LED task");

    if (task_handle != NULL) {
        eTaskState task_state = eTaskGetState(task_handle);
        printf("LED_Task state: %d\n", task_state);
        // States are:
        // 0 = Running
        // 1 = Ready
        // 2 = Blocked
        // 3 = Suspended
        // 4 = Deleted (but not yet cleaned up)
    } else {
        printf("Couldn't find LED_Task!\n");
    }
#if 1


    vTaskList(buf);
    printf("\r\nTask List:\r\n");
    printf("Name          State  Priority  Stack   Num\r\n");
    printf("*******************************************\r\n");
    printf("%s\r\n", buf);



    // Alternative: just get number of tasks
    UBaseType_t num_tasks = uxTaskGetNumberOfTasks();
    printf("Number of tasks: %u\n", num_tasks);


    // Add this debugging code before vTaskStartScheduler
    volatile uint32_t *sys_tick_ctrl = (uint32_t *)0xE000E010;
    volatile uint32_t *sys_tick_load = (uint32_t *)0xE000E014;
    volatile uint32_t *sys_tick_val  = (uint32_t *)0xE000E018;

    printf("SysTick CTRL: 0x%08lx\n", *sys_tick_ctrl);
    printf("SysTick LOAD: 0x%08lx\n", *sys_tick_load);
    printf("SysTick VAL:  0x%08lx\n", *sys_tick_val);

extern void xPortPendSVHandler( void );
extern void vPortSVCHandler( void );
extern void xPortSysTickHandler( void );

printf("PendSV Handler: %p\n", xPortPendSVHandler);
printf("SVC Handler: %p\n", vPortSVCHandler);
printf("SysTick Handler: %p\n", xPortSysTickHandler);


#endif
    printf("ABOUT TO START SCHEDULER\n");
    vTaskStartScheduler();
    printf("SCHEDULER STARTED\n");
    while(1)
    {
        configASSERT(0);
    }
}




void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("Stack overflow in task: %s\n", pcTaskName);
    while(1); // Halt for debugging
}

void vApplicationMallocFailedHook(void) {
    printf("Malloc failed!\n");
    while(1); // Halt for debugging
}

void vApplicationTickHook(void) {
    pico_set_led(true);
    // static uint32_t tick_count = 0;
    // if(++tick_count % configTICK_RATE_HZ == 0) {  // Print every second
    //     printf("Tick: %lu\n", tick_count);
    // }
}

void vApplicationIdleHook(void) {
}
