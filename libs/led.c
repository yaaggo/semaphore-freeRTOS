#include "led.h"

// função estática para configurar o led como pwm
static uint pwm_setup(uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 255);
    pwm_init(slice_num, &config, true);

    return slice_num;
}
// inicializ o led, porem desligado
void led_init(uint8_t pin) {
    pwm_setup(pin);
    led_intensity(pin, 0);
}
// função que coloca a intensidade no led
void led_intensity(uint8_t pin, uint8_t intensity) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint channel = pwm_gpio_to_channel(pin);

    pwm_set_chan_level(slice_num, channel, intensity);

    pwm_set_enabled(slice_num, true);
}

void led_clear() {
    led_intensity(LED_BLUE_PIN, 0);
    led_intensity(LED_RED_PIN, 0);
    led_intensity(LED_GREEN_PIN, 0);
}