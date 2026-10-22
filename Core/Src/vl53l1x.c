#include "vl53l1x.h"

// Tabella di configurazione base validata ST (Short Distance Mode, 1.3 m max)
static const uint8_t VL53L1X_DEFAULT_CONFIG[91] = {
    0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x02, 0x08, 0x00, 0x08,
    0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0b, 0x00, 0x00, 0x02,
    0x14, 0x21, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xc8,
    0x00, 0x00, 0x38, 0xff, 0x01, 0x00, 0x08, 0x00, 0x00, 0x01,
    0xcc, 0x0f, 0x01, 0xf1, 0x0d, 0x01, 0x68, 0x00, 0x80, 0x08,
    0xb8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x89, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x0f, 0x0d, 0x0e, 0x0e, 0x00,
    0x00, 0x02, 0xc7, 0xff, 0x9B, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x40
};

// 1. Lettura registro identificativo del sensore
uint16_t VL53L1X_CheckID(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[2] = {0, 0};
    if (HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x010F, I2C_MEMADD_SIZE_16BIT, buf, 2, 100) == HAL_OK) {
        return ((uint16_t)buf[0] << 8) | buf[1];
    }
    return 0;
}

// 2. Inizializzazione lineare e pulita
uint8_t VL53L1X_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t boot_state = 0;
    uint32_t start_tick = HAL_GetTick();

    // Attesa completamento boot interno al sensore
    while ((boot_state & 0x01) == 0) {
        HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x00E5, I2C_MEMADD_SIZE_16BIT, &boot_state, 1, 100);
        if ((HAL_GetTick() - start_tick) > 500) return 0;
        HAL_Delay(2);
    }

    // Caricamento configurazione standard a partire dal registro 0x002D
    if (HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x002D, I2C_MEMADD_SIZE_16BIT,
                          (uint8_t*)VL53L1X_DEFAULT_CONFIG, 91, 200) != HAL_OK) {
        return 0;
    }

    HAL_Delay(20);

    // Reset interrupt e avvio misurazione continua
    uint8_t clr = 0x01;
    uint8_t start = 0x40;
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x0086, I2C_MEMADD_SIZE_16BIT, &clr, 1, 100);
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x0087, I2C_MEMADD_SIZE_16BIT, &start, 1, 100);

    return 1;
}

// 3. Controllo presenza nuova misura
uint8_t VL53L1X_DataReady(I2C_HandleTypeDef *hi2c) {
    uint8_t status = 0;
    // Legge il registro GPIO__TIO_HV_STATUS
    if (HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0031, I2C_MEMADD_SIZE_16BIT, &status, 1, 10) == HAL_OK) {
        return (status & 0x01);
    }
    return 0;
}

// 4. Lettura pura del valore in mm
uint16_t VL53L1X_GetDistanceMM(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[2] = {0, 0};
    uint8_t clr = 0x01;

    // Lettura valore grezzo di distanza (16 bit Big Endian da 0x0096)
    if (HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0096, I2C_MEMADD_SIZE_16BIT, buf, 2, 20) != HAL_OK) {
        return 0;
    }

    // Riarmo interrupt per avviare il campionamento successivo
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, 0x0086, I2C_MEMADD_SIZE_16BIT, &clr, 1, 20);

    return ((uint16_t)buf[0] << 8) | buf[1];
}
