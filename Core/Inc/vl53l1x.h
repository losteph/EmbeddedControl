/*
 * vl53l1x.h
 *
 *  Created on: 22 ott 2026
 *      Author: Steph
 */

#ifndef INC_VL53L1X_H_
#define INC_VL53L1X_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define VL53L1X_ADDR  (0x29 << 1) // 0x52 per le API HAL I2C

uint16_t VL53L1X_CheckID(I2C_HandleTypeDef *hi2c);
uint8_t  VL53L1X_Init(I2C_HandleTypeDef *hi2c);
uint8_t  VL53L1X_SetROI(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y);
uint8_t  VL53L1X_DataReady(I2C_HandleTypeDef *hi2c);
uint16_t VL53L1X_GetDistanceMM(I2C_HandleTypeDef *hi2c);

#endif /* INC_VL53L1X_H_ */
