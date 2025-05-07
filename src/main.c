#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "../libs/display.h"
#include "../libs/button.h"
#include "../libs/matrix.h"
#include "../libs/led.h"
#include "../libs/buzzer.h"

#define TASK_DELAY_MS 100

typedef enum {
    VERDE,
    AMARELO,
    VERMELHO,
    PISCANDO
} EstadoSemaforo;

void setup();

void task_mode(void *pvParameters);
void task_semaphore(void *pvParameters);
void task_buzzer(void *pvParameters);
void task_matrix(void *pvParameters);
void task_display(void *pvParameters);

void buzzer_beep(uint8_t gpio, uint16_t freq, uint16_t duration_ms);
bool delay_interrompivel(int tempo_ms, bool modo_original);

display dp;

volatile int noturn_mode = 0;
volatile EstadoSemaforo estado_atual = VERDE;


int main() {
    stdio_init_all();
    setup();

    xTaskCreate (
        task_mode, 
        "Mode",
        configMINIMAL_STACK_SIZE,
        NULL,
        2,
        NULL
    );

    xTaskCreate (
        task_semaphore, 
        "Semaphore",
        configMINIMAL_STACK_SIZE,
        NULL,
        1,
        NULL
    );

    xTaskCreate (
        task_buzzer, 
        "Buzzer",
        configMINIMAL_STACK_SIZE,
        NULL,
        1,
        NULL
    );

    xTaskCreate (
        task_matrix, 
        "Matrix",
        configMINIMAL_STACK_SIZE,
        NULL,
        1,
        NULL
    );
    xTaskCreate (
        task_display, 
        "Display",
        configMINIMAL_STACK_SIZE,
        NULL,
        1,
        NULL
    );

    vTaskStartScheduler();
    panic_unsupported();
}

void setup() {
    display_init(&dp);
    matrix_init(MATRIX_LED_PIN);
    button_init();
    buzzer_init(BUZZER_A_PIN);
    led_init(LED_BLUE_PIN);
    led_init(LED_RED_PIN);
    led_init(LED_GREEN_PIN);
}

bool delay_interrompivel(int tempo_ms, bool modo_original) {
    for (int i = 0; i < tempo_ms; i += 100) {
        if (noturn_mode != modo_original) return false;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return true;
}

void task_mode(void *pvParameters) {
    while (1) {
        button_event button_state = button_get_event();

        if (button_state != BUTTON_NONE) {
            if (button_state == BUTTON_A) {
                noturn_mode = !noturn_mode;
                estado_atual = noturn_mode ? PISCANDO : VERDE;
            } else if (button_state == BUTTON_B) {
                display_shutdown(&dp);
                matrix_clear();
                matrix_update();
                reset_usb_boot(0, 0);
            }
            button_clear_event();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task_semaphore(void *pvParameters) {
    while (1) {
        if (noturn_mode) {
            estado_atual = PISCANDO;
            led_clear();
            led_intensity(LED_RED_PIN, 150);
            led_intensity(LED_GREEN_PIN, 150);
            if (!delay_interrompivel(500, true)) continue;

            led_clear();
            if (!delay_interrompivel(1500, true)) continue;

        } else {
            estado_atual = VERDE;
            led_clear();
            led_intensity(LED_GREEN_PIN, 150);
            if (!delay_interrompivel(5000, false)) continue;

            estado_atual = AMARELO;
            led_clear();
            led_intensity(LED_RED_PIN, 150);
            led_intensity(LED_GREEN_PIN, 150);
            if (!delay_interrompivel(2000, false)) continue;

            estado_atual = VERMELHO;
            led_clear();
            led_intensity(LED_RED_PIN, 150);
            if (!delay_interrompivel(3000, false)) continue;
        }
    }
}

void task_display(void *pvParameters) {
    bool blink_on = true;

    while (1) {
        display_clear(&dp);

        display_draw_rectangle(0, 0, 20, 20, estado_atual == VERDE, true, &dp);
        display_draw_rectangle(0, 22, 20, 42, (estado_atual == AMARELO || estado_atual == PISCANDO), true, &dp);
        display_draw_rectangle(0, 44, 20, 63, estado_atual == VERMELHO, true, &dp);

        if (estado_atual == VERDE) {
            display_draw_string(25, 4, "VERDE", true, &dp);
        } else if (estado_atual == AMARELO) {
            display_draw_string(25, 26, "AMARELO", true, &dp);
        } else if (estado_atual == VERMELHO) {
            display_draw_string(25, 48, "VERMELHO", true, &dp);
        } else if (estado_atual == PISCANDO) {
            if (blink_on) {
                display_draw_string(25, 26, "AMARELO", true, &dp);
            }
        }

        display_draw_string(70, 0, noturn_mode ? "NOTURNO" : "NORMAL", true, &dp);

        display_update(&dp);

        if (noturn_mode) {
            blink_on = !blink_on;
            if (!delay_interrompivel(500, true)) continue;
        } else {
            if (!delay_interrompivel(100, false)) continue;
        }
    }
}


void task_matrix(void *pvParameters) {
    bool on = true;

    while (1) {
        matrix_clear();

        switch (estado_atual) {

            case VERDE:
                for (int i = 1; i < 4; i++) {
                    for (int j = 1; j < 4; j++) {
                        matrix_set_led_xy(j, i, COLOR_RGB(0, 60, 0));
                    }
                }
                matrix_update();
                vTaskDelay(pdMS_TO_TICKS(100));
                break;

            case AMARELO:
                for (int i = 1; i < 4; i++) {
                    for (int j = 1; j < 4; j++) {
                        matrix_set_led_xy(j, i, COLOR_RGB(60, 60, 0));
                    }
                }
                matrix_update();
                vTaskDelay(pdMS_TO_TICKS(100));
                break;

            case VERMELHO:
                for (int i = 1; i < 4; i++) {
                    for (int j = 1; j < 4; j++) {
                        matrix_set_led_xy(j, i, COLOR_RGB(60, 0, 0));
                    }
                }
                matrix_update();
                vTaskDelay(pdMS_TO_TICKS(100));
                break;

            case PISCANDO:
                if (on) {
                    for (int i = 1; i < 4; i++) {
                        for (int j = 1; j < 4; j++) {
                            matrix_set_led_xy(j, i, COLOR_RGB(120, 120, 0));
                        }
                    }
                } else {
                    matrix_clear();
                }
                matrix_update();
                on = !on;
                vTaskDelay(pdMS_TO_TICKS(500));
                break;
        }
    }
}


void buzzer_rtos_beep(uint8_t gpio, uint16_t freq, uint16_t duration_ms) {
    buzzer_turn_on(gpio, freq);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    buzzer_turn_off(gpio);
}

void task_buzzer(void *pvParameters) {
    while (1) {
        switch (estado_atual) {
            case VERDE:
                buzzer_rtos_beep(BUZZER_A_PIN, 1000, 500);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case AMARELO:
                for (int i = 0; i < 4; i++) {
                    buzzer_rtos_beep(BUZZER_A_PIN, 1000, 100);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                break;

            case VERMELHO:
                buzzer_rtos_beep(BUZZER_A_PIN, 1000, 500);
                vTaskDelay(pdMS_TO_TICKS(1500));
                break;

            case PISCANDO:
                buzzer_rtos_beep(BUZZER_A_PIN, 1000, 100);
                vTaskDelay(pdMS_TO_TICKS(1900));
                break;
        }
    }
}
