# Levitating Ball 

Il sistema realizza il controllo ad anello chiuso per stabilizzare e regolare la quota di levitazione di una pallina all'interno di un tubo trasparente. 

La spinta verticale è modulata tramite una ventola a 12V pilotata in PWM, mentre l'altezza reale viene acquisita da un sensore laser Time-of-Flight (VL53L1X) posizionato in cima alla colonna. La quota di riferimento (setpoint) può essere modificata manualmente tramite due pulsanti esterni.

👉 **[Video Dimostrativo](./demo_video.mp4)**

---

## Hardware Utilizzato

* **Scheda di sviluppo:** STM32 NUCLEO-F446RE (ARM Cortex-M4 @ 180 MHz)
* **Sensore di distanza:** Modulo laser ottico Time-of-Flight VL53L1X 
* **Attuatore:** Ventola ad alta velocità TUNGFULL (12V DC, 2.55A)
* **Stadio di potenza:** Modulo MOSFET LR7843 con optoisolatore
* **Input utente:** Potenziometro lineare per la gestione del setpoint dinamico

---

## Mappatura Pin Principali

* `PA5`: Uscita PWM per il gate del MOSFET (TIM2_CH1)
* `PB8`: Segnale SCL per la comunicazione I2C1 con il sensore ToF
* `PB9`: Segnale SDA per la comunicazione I2C1 con il sensore ToF
* `PA0`: Ingresso analogico ADC1 (ADC1_IN0) per la lettura del potenziometro di setpoint

---

## Struttura del Repository

- `Core/Src/` e `Core/Inc/`: Codice sorgente firmware (controllore PID, driver sensore, gestione ADC e main).
- `EmbeddedControl.ioc`: File di configurazione hardware per STM32CubeMX.
