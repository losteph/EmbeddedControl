#include "vl53l1x_user.h"

// Funzioni statiche di supporto per registri a 16 bit
static HAL_StatusTypeDef VL53L1X_WriteReg(VL53L1X_Dev *dev, uint16_t reg, uint8_t val) {
    return HAL_I2C_Mem_Write(dev->hi2c, VL53L1X_I2C_ADDR, reg, I2C_MEMADD_SIZE_16BIT, &val, 1, VL53L1X_I2C_TIMEOUT);
}

static HAL_StatusTypeDef VL53L1X_ReadReg16(VL53L1X_Dev *dev, uint16_t reg, uint16_t *val) {
    uint8_t buf[2];
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(dev->hi2c, VL53L1X_I2C_ADDR, reg, I2C_MEMADD_SIZE_16BIT, buf, 2, VL53L1X_I2C_TIMEOUT);
    if (status == HAL_OK) {
        *val = (uint16_t)((buf[0] << 8) | buf[1]);
    }
    return status;
}

HAL_StatusTypeDef VL53L1X_Init(VL53L1X_Dev *dev, I2C_HandleTypeDef *hi2c_handle) {
    dev->hi2c = hi2c_handle;
    dev->last_distance_mm = 0;
    dev->is_initialized = 0;

    uint16_t model_id = 0;
    // Verifica presenza hardware leggendo Model ID (atteso: 0xEACC)
    if (VL53L1X_ReadReg16(dev, VL53L1_IDENTIFICATION__MODEL_ID, &model_id) != HAL_OK) {
        return HAL_ERROR;
    }

    if (model_id != 0xEACC) {
        return HAL_ERROR; // Dispositivo sconosciuto sul bus
    }

    dev->is_initialized = 1;
    return HAL_OK;
}

HAL_StatusTypeDef VL53L1X_StartRanging(VL53L1X_Dev *dev) {
    if (!dev->is_initialized) return HAL_ERROR;
    // Avvia la misurazione continua scrivendo 0x40 nel registro SYSTEM__MODE_START
    return VL53L1X_WriteReg(dev, VL53L1_SYSTEM__MODE_START, 0x40);
}

uint16_t VL53L1X_ReadDistance(VL53L1X_Dev *dev) {
    if (!dev->is_initialized) return 0;

    uint16_t raw_dist = 0;
    // Lettura della misura a 16 bit
    if (VL53L1X_ReadReg16(dev, VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0, &raw_dist) == HAL_OK) {
        // Clear interrupt hardware del sensore per abilitare il prossimo campione
        VL53L1X_WriteReg(dev, VL53L1_SYSTEM__INTERRUPT_CLEAR, 0x01);

        // Filtraggio grossolano degli outlier fuori range fisico del tubo
        if (raw_dist > 20 && raw_dist < 1000) {
            dev->last_distance_mm = raw_dist;
        }
    }
    return dev->last_distance_mm;
}
