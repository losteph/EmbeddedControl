# Levitating Ball 

Il sistema realizza il controllo ad anello chiuso per stabilizzare e regolare la quota di levitazione di una pallina all'interno di un tubo trasparente. 

La spinta verticale è modulata tramite una ventola a 12V pilotata in PWM, mentre l'altezza reale viene acquisita da un sensore laser Time-of-Flight (VL53L1X) posizionato in cima alla colonna. La quota di riferimento (setpoint) può essere modificata manualmente tramite due pulsanti esterni.

---

## Video Dimostrativo

<video src="docs/demo_video.mp4" controls="controls" width="100%">
  Il tuo browser non supporta il tag video. Puoi scaricare il file direttamente da [docs/demo_video.mp4](docs/demo_video.mp4).
</video>

---

## Hardware Utilizzato

* **Scheda di sviluppo:** STM32 NUCLEO-F446RE (ARM Cortex-M4 @ 180 MHz)
* **Sensore di distanza:** Modulo laser ottico Time-of-Flight VL53L1X 
* **Attuatore:** Ventola ad alta velocità TUNGFULL (12V DC, 2.55A)
* **Stadio di potenza:** Modulo MOSFET LR7843 con optoisolatore
* **Input utente:** 2 pulsanti normalmente aperti collegati su linee di interrupt (EXTI)

---

## Mappatura Pin Principali

* `PA5`: Uscita PWM per il gate del MOSFET (TIM2_CH1)

---

## Struttura del Repository

* `Core/Src/` e `Core/Inc/`: Codice sorgente firmware (controllore `pid.c`, driver sensore `vl53l1x_user.c`, gestione pulsanti `buttons.c` e `main.c`).
* `EmbeddedControl.ioc`: File di configurazione hardware per STM32CubeMX.
