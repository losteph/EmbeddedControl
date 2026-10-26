#ifndef INC_POTENTIOMETER_H_
#define INC_POTENTIOMETER_H_

#include "main.h"

#define ADC_BUF_LEN 16

// Inizializza l'avvio hardware del DMA collegato all'ADC
void Potentiometer_Init(ADC_HandleTypeDef *hadc);

// Legge il buffer DMA, calcola la media, applica i limiti e il filtro di deadband
float Potentiometer_GetSetpoint(float min_val, float max_val);

#endif /* INC_POTENTIOMETER_H_ */
