#include "pico/stdlib.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

int pico_led_init(void) {
#ifdef CYW43_WL_GPIO_LED_PIN
    return cyw43_arch_init();
#else
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
}

void pico_set_led(bool led_on) {
#ifdef CYW43_WL_GPIO_LED_PIN
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#else
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#endif
}

