#include <stdio.h>
#include <array>
#include "pico/stdlib.h"
#include "GPIO.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/cyw43_arch.h"


// static pico_cpp::GPIO_Pin ledPin(25, pico_cpp::PinType::Output);


// Perform initialisation
int pico_led_init(void) {
    return cyw43_arch_init();
}

// Turn the led on or off
void pico_set_led(bool led_on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}


#if 0
void vTaskCode(void *pvParameters)
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    */
    for(;;)
    {
        // ledPin.set_high();
        vTaskDelay(1000);
        // ledPin.set_low();
        // vTaskDelay(1000);
    }
}
#endif

int main() {
    pico_set_led(true);
    for(;;);

#if 0
BaseType_t xReturned;
TaskHandle_t xHandle = NULL;
/* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    vTaskCode,       /* Function that implements the task. */
                    "Blinky task",   /* Text name for the task. */
                    512,             /* Stack size in words, not bytes. */
                    ( void * ) 1,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,/* Priority at which the task is created. */
                    &xHandle );   

    vTaskStartScheduler();
    while(1)
    {
        configASSERT(0);    /* We should never get here */
    }
#endif
}