/*
 * vl53l1x.c - Con Reset Offset Interni, Piena Matrice SPAD e Calibrazione Lineare
 */

#include "vl53l1x.h"

static const uint8_t VL53L1X_DEFAULT_CONFIG[91] = {
    0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x02, 0x08, 0x00, 0x08,
    0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0b, 0x00, 0x00, 0x02,
    0x0a, 0x21, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xc8,
    0x00, 0x00, 0x38, 0xff, 0x01, 0x00, 0x08, 0x00, 0x00, 0x01,
    0xcc, 0x0f, 0x01, 0xf1, 0x0d, 0x01, 0x68, 0x00, 0x80, 0x08,
    0xb8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x89, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x0f, 0x0d, 0x0e, 0x0e, 0x00,
    0x00, 0x02, 0xc7, 0xff, 0x9B, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x40
};

static void WriteReg8(I2C_HandleTypeDef *hi2c, uint16_t reg, uint8_t val) {
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, reg, I2C_MEMADD_SIZE_16BIT, &val, 1, 50);
}

static void WriteReg16(I2C_HandleTypeDef *hi2c, uint16_t reg, uint16_t val) {
    uint8_t buf[2] = { (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, reg, I2C_MEMADD_SIZE_16BIT, buf, 2, 50);
}

static void WriteReg32(I2C_HandleTypeDef *hi2c, uint16_t reg, uint32_t val) {
    uint8_t buf[4] = {
        (uint8_t)(val >> 24), (uint8_t)(val >> 16),
        (uint8_t)(val >> 8),  (uint8_t)(val & 0xFF)
    };
    HAL_I2C_Mem_Write(hi2c, VL53L1X_ADDR, reg, I2C_MEMADD_SIZE_16BIT, buf, 4, 50);
}

uint16_t VL53L1X_CheckID(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[2] = {0, 0};
    if (HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x010F, I2C_MEMADD_SIZE_16BIT, buf, 2, 100) == HAL_OK) {
        return ((uint16_t)buf[0] << 8) | buf[1];
    }
    return 0;
}

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

    // Prima calibrazione VHV all'avvio
    WriteReg8(hi2c, 0x0087, 0x40);
    start_tick = HAL_GetTick();
    while (!VL53L1X_DataReady(hi2c)) {
        if ((HAL_GetTick() - start_tick) > 500) return 0;
        HAL_Delay(2);
    }
    WriteReg8(hi2c, 0x0086, 0x01);
    WriteReg8(hi2c, 0x0087, 0x00);
    WriteReg8(hi2c, 0x0008, 0x09);
    WriteReg8(hi2c, 0x000B, 0x00);

    // AZZERA OFFSET INTERNI E CROSSTALK (che sui cloni cinesi sballano lo zero!)
    WriteReg16(hi2c, 0x001E, 0x0000); // ALGO__PART_TO_PART_RANGE_OFFSET_MM
    WriteReg16(hi2c, 0x0020, 0x0000); // MM_CONFIG__INNER_OFFSET_MM
    WriteReg16(hi2c, 0x0022, 0x0000); // MM_CONFIG__OUTER_OFFSET_MM
    WriteReg16(hi2c, 0x0016, 0x0000); // ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS

    // Imposta SHORT DISTANCE MODE
    WriteReg8(hi2c,  0x004B, 0x14);
    WriteReg8(hi2c,  0x0060, 0x07);
    WriteReg8(hi2c,  0x0063, 0x05);
    WriteReg8(hi2c,  0x0069, 0x38);
    WriteReg16(hi2c, 0x0078, 0x0705);
    WriteReg16(hi2c, 0x007A, 0x0606);

    // Imposta TIMING BUDGET = 33 ms in Short Mode (molto più stabile di 20 ms!)
    WriteReg16(hi2c, 0x005E, 0x02D6); // Timeout A per 33ms Short
    WriteReg16(hi2c, 0x0062, 0x02E4); // Timeout B per 33ms Short

    // Calibra Inter-Measurement a 25 ms (40 Hz)
    uint8_t pll_buf[2] = {0, 0};
    HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x00DE, I2C_MEMADD_SIZE_16BIT, pll_buf, 2, 50);
    uint16_t clock_pll = (((uint16_t)pll_buf[0] << 8) | pll_buf[1]) & 0x03FF;
    if (clock_pll == 0) clock_pll = 0x0289; // Fallback di sicurezza
    uint32_t inter_meas = (uint32_t)((float)clock_pll * 25.0f * 1.075f);
    WriteReg32(hi2c, 0x006C, inter_meas);

    // Usa ROI 8x8 al centro (compromesso perfetto: stringe il cono ma prende 4 volte più luce di 4x4!)
    WriteReg8(hi2c, 0x007F, 199);
    WriteReg8(hi2c, 0x0080, 0x77); // 8x8 SPAD

    // Avvia Ranging continuo
    WriteReg8(hi2c, 0x0086, 0x01);
    WriteReg8(hi2c, 0x0087, 0x40);

    return 1;
}

uint8_t VL53L1X_SetROI(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y) {
    if (x < 4) x = 4;
    if (y < 4) y = 4;
    if (x > 16) x = 16;
    if (y > 16) y = 16;
    WriteReg8(hi2c, 0x007F, 199);
    WriteReg8(hi2c, 0x0080, ((y - 1) << 4) | (x - 1));
    return 1;
}

uint8_t VL53L1X_DataReady(I2C_HandleTypeDef *hi2c) {
    uint8_t hv_mux = 0, status = 0;
    HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0030, I2C_MEMADD_SIZE_16BIT, &hv_mux, 1, 10);
    HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0031, I2C_MEMADD_SIZE_16BIT, &status, 1, 10);
    uint8_t int_pol = ((hv_mux & 0x10) >> 4) ^ 1;
    return ((status & 0x01) == int_pol);
}

// Variabile statica per filtrare le piccole oscillazioni (Low-Pass Filter)
static float filtered_mm = 0.0f;

uint16_t VL53L1X_GetDistanceMM(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[2] = {0, 0};
    HAL_I2C_Mem_Read(hi2c, VL53L1X_ADDR, 0x0096, I2C_MEMADD_SIZE_16BIT, buf, 2, 20);
    WriteReg8(hi2c, 0x0086, 0x01);

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];

    // Se volete vedere il dato grezzo senza retta, lasciate: float real_mm = (float)raw;
    // Se invece anche dopo l'azzeramento di 0x001E legge ancora con la retta di prima,
    // questa è l'equazione esatta ricavata dai vostri punti (15->45mm, 35->80mm, 45->100mm):
    float real_mm = 0.0f;
    if (raw > 0) {
        real_mm = (1.85f * (float)raw) + 17.0f;
    }

    // Filtro passa-basso esponenziale (70% misura nuova + 30% precedente) per eliminare il tremolio!
    if (filtered_mm == 0.0f) {
        filtered_mm = real_mm;
    } else {
        filtered_mm = (0.7f * real_mm) + (0.3f * filtered_mm);
    }

    return (uint16_t)(filtered_mm + 0.5f);
}
