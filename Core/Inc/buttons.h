#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include "main.h"

#define DEBOUNCE_DELAY_MS   150
#define SETPOINT_STEP_MM    10.0f
#define SETPOINT_MIN_MM     50.0f
#define SETPOINT_MAX_MM     350.0f

void Buttons_HandleEXTI(uint16_t GPIO_Pin, float *setpoint);

#endif /* INC_BUTTONS_H_ */
