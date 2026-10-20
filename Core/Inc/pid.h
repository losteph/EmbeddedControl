#ifndef INC_PID_H_
#define INC_PID_H_

#include <stdint.h>

typedef struct {
    // Guadagni di controllo
    float Kp;
    float Ki;
    float Kd;

    // Parametri temporali
    float Ts;            // Tempo di campionamento (secondi, es. 0.02f)

    // Limiti dell'attuatore (Duty Cycle Timer CCR)
    float out_min;       // Valore minimo (es. 0.0f)
    float out_max;       // Valore massimo (es. 4499.0f)

    // Memoria interna
    float integrator;    // Stato dell'integratore
    float prev_error;    // Errore al passo k-1
} PID_Controller;

void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float Ts, float out_min, float out_max);
float PID_Update(PID_Controller *pid, float setpoint, float measurement);
void PID_Reset(PID_Controller *pid);

#endif /* INC_PID_H_ */
