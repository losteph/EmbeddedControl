#include "buttons.h"

static uint32_t last_press_time_btn1 = 0;
static uint32_t last_press_time_btn2 = 0;

void Buttons_HandleEXTI(uint16_t GPIO_Pin, float *setpoint) {
    uint32_t current_time = HAL_GetTick();

    // Pulsante 1: Incremento Setpoint Quota (PC0 - EXTI0)
    if (GPIO_Pin == GPIO_PIN_0) {
        if ((current_time - last_press_time_btn1) > DEBOUNCE_DELAY_MS) {
            *setpoint += SETPOINT_STEP_MM;
            if (*setpoint > SETPOINT_MAX_MM) {
                *setpoint = SETPOINT_MAX_MM;
            }
            last_press_time_btn1 = current_time;
        }
    }
    // Pulsante 2: Decremento Setpoint Quota (PC1 - EXTI1)
    else if (GPIO_Pin == GPIO_PIN_1) {
        if ((current_time - last_press_time_btn2) > DEBOUNCE_DELAY_MS) {
            *setpoint -= SETPOINT_STEP_MM;
            if (*setpoint < SETPOINT_MIN_MM) {
                *setpoint = SETPOINT_MIN_MM;
            }
            last_press_time_btn2 = current_time;
        }
    }
}
