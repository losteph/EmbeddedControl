#include "potentiometer.h"

// Buffer circolare DMA isolato all'interno del modulo
static uint16_t adc_dma_buffer[ADC_BUF_LEN];

static const float V_REF = 3.30f;
static const float ADC_MAX_VAL = 4095.0f;
static const float SETPOINT_NOISE_THRESHOLD = 0.8f; // Isteresi anti-rumore

static float current_setpoint = 30.0f; // Valore memorizzato per il filtro

void Potentiometer_Init(ADC_HandleTypeDef *hadc)
{
    // Avvia il trasferimento circolare DMA in background
    HAL_ADC_Start_DMA(hadc, (uint32_t*)adc_dma_buffer, ADC_BUF_LEN);
}

float Potentiometer_GetSetpoint(float min_val, float max_val)
{
    // 1. Calcola la media dei 16 campioni
    uint32_t adc_sum = 0;
    for (int i = 0; i < ADC_BUF_LEN; i++)
    {
        adc_sum += adc_dma_buffer[i];
    }
    float raw_filtered = (float)adc_sum / (float)ADC_BUF_LEN;

    // 2. Converte in tensione
    float voltage = (raw_filtered * V_REF) / ADC_MAX_VAL;

    // 3. Mappatura lineare sul range desiderato
    float new_setpoint = min_val + ((voltage / V_REF) * (max_val - min_val));

    // 4. Deadband agli estremi (fine corsa pulito)
    if (new_setpoint < (min_val + 1.0f)) new_setpoint = min_val;
    if (new_setpoint > (max_val - 1.0f)) new_setpoint = max_val;

    // 5. Filtro di isteresi: aggiorna solo per variazioni superiori alla soglia di rumore
    float delta = new_setpoint - current_setpoint;
    if (delta < 0.0f) delta = -delta;

    if (delta >= SETPOINT_NOISE_THRESHOLD || new_setpoint == min_val || new_setpoint == max_val)
    {
        current_setpoint = new_setpoint;
    }

    return current_setpoint;
}
