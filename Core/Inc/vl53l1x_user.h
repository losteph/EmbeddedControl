#ifndef INC_VL53L1X_USER_H_
#define INC_VL53L1X_USER_H_

#include "main.h"
#include <stdint.h>

#define VL53L1X_I2C_ADDR       (0x29 << 1) // Indirizzo I2C 8-bit (0x52)
#define VL53L1X_I2C_TIMEOUT    10          // Timeout HAL in ms

// Registri fondamentali VL53L1X
#define VL53L1_IDENTIFICATION__MODEL_ID         0x010F
#define VL53L1_SYSTEM__INTERRUPT_CLEAR          0x0086
#define VL53L1_SYSTEM__MODE_START               0x0087
#define VL53L1_RESULT__RANGE_STATUS             0x0089
#define VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0 0x0096

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t last_distance_mm;
    uint8_t is_initialized;
} VL53L1X_Dev;

// Prototipi funzioni
HAL_StatusTypeDef VL53L1X_Init(VL53L1X_Dev *dev, I2C_HandleTypeDef *hi2c_handle);
HAL_StatusTypeDef VL53L1X_StartRanging(VL53L1X_Dev *dev);
uint16_t VL53L1X_ReadDistance(VL53L1X_Dev *dev);

#endif /* INC_VL53L1X_USER_H_ */
