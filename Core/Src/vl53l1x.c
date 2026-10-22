/*
 * vl53l1x.c
 *
 *  Created on: 22 ott 2026
 *      Author: Steph
 */


#include "vl53l1x.h"

// Tabella di configurazione standard ULD ST (Short Mode, 20 ms timing budget)
static const uint8_t VL53L1X_DEFAULT_CONFIG[91] = {
    0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x02, 0x08, 0x00, 0x08,
    0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0b, 0x00, 0x00, 0x02,
    0x14, 0x21, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xc8,
    0x00, 0x00, 0x38, 0xff, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
    0xad, 0x07, 0x00, 0x00, 0xb5, 0x05, 0xb8, 0x00, 0xbb, 0x08,
    0x38, 0x00, 0x00, 0x00, 0x00, 0x02, 0x89, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x05, 0x06, 0x06, 0x00,
    0x00, 0x02, 0xc7, 0xff, 0x9B, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x40
};

// 1. Lettura registro WHO_AM_I (0x010F): valore atteso 0xEACC
uint16_t VL53L1X_CheckID(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[2] = {0, 0};
    if (HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x010F, I2C_MEMADD_SIZE_16BIT, buf, 2, 100) == HAL_OK) {
        return ((uint16_t)buf[0] << 8) | buf[1];
    }
    return 0;
}

// 2. Inizializzazione sensore (Short mode 20 ms)
uint8_t VL53L1X_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t boot_state = 0;
    uint32_t start_tick = HAL_GetTick();

    while ((boot_state & 0x01) == 0) {
        HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x00E5, I2C_MEMADD_SIZE_16BIT, &boot_state, 1, 100);
        if ((HAL_GetTick() - start_tick) > 500) return 0;
        HAL_Delay(2);
    }

    if (HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x002D, I2C_MEMADD_SIZE_16BIT,
                          (uint8_t*)VL53L1X_DEFAULT_CONFIG, 91, 200) != HAL_OK) {
        return 0;
    }

    HAL_Delay(50);

    uint8_t clr = 0x01, start = 0x40;
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x0086, I2C_MEMADD_SIZE_16BIT, &clr, 1, 100);
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x0087, I2C_MEMADD_SIZE_16BIT, &start, 1, 100);

    return 1;
}

// 3. Restringe il campo visivo (FoV): x=4, y=4 -> area 4x4 SPAD (~15 gradi)
uint8_t VL53L1X_SetROI(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y) {
    if (x < 4) x = 4;
    if (y < 4) y = 4;
    if (x > 16) x = 16;
    if (y > 16) y = 16;

    // Centro ottico standard della matrice 16x16: 199
    uint8_t center = 199;
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x006E, I2C_MEMADD_SIZE_16BIT, &center, 1, 100);

    // Finestra ROI: formato (Larghezza - 1) e (Altezza - 1)
    uint8_t roi_cfg[2];
    roi_cfg[0] = ((y - 1) << 4) | (x - 1);
    roi_cfg[1] = 0x00;
    return (HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x006F, I2C_MEMADD_SIZE_16BIT, roi_cfg, 2, 100) == HAL_OK);
}

// 4. Polling data ready
uint8_t VL53L1X_DataReady(I2C_HandleTypeDef *hi2c) {
    uint8_t status = 0;
    if (HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0031, I2C_MEMADD_SIZE_16BIT, &status, 1, 10) == HAL_OK) {
        return (status & 0x01);
    }
    return 0;
}

// 5. Lettura distanza in mm e ri-armo
uint16_t VL53L1X_GetDistanceMM(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[2] = {0, 0};
    uint8_t clr = 0x01;

    HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0096, I2C_MEMADD_SIZE_16BIT, buf, 2, 10);
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x0086, I2C_MEMADD_SIZE_16BIT, &clr, 1, 10);

    return ((uint16_t)buf[0] << 8) | buf[1];
}
