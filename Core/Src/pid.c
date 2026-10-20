#include "pid.h"

void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float Ts, float out_min, float out_max) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->Ts = Ts;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
}

void PID_Reset(PID_Controller *pid) {
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
}

float PID_Update(PID_Controller *pid, float setpoint, float measurement) {
    // 1. Calcolo dell'errore di inseguimento e[k]
    float error = setpoint - measurement;

    // 2. Azione Proporzionale
    float P = pid->Kp * error;

    // 3. Azione Integrale provvisoria (Eulero in avanti/rettangolare)
    float tentative_integrator = pid->integrator + (pid->Ki * error * pid->Ts);

    // 4. Azione Derivativa su errore (Differenza all'indietro)
    float D = pid->Kd * ((error - pid->prev_error) / pid->Ts);

    // 5. Uscita provvisoria prima della saturazione
    float output = P + tentative_integrator + D;

    // 6. Saturazione e Anti-Windup Clamping
    if (output > pid->out_max) {
        output = pid->out_max;
        // Non accumulare l'integrale se l'errore spinge ancora oltre il limite massimo
        if (error < 0.0f) {
            pid->integrator = tentative_integrator;
        }
    } else if (output < pid->out_min) {
        output = pid->out_min;
        // Non accumulare l'integrale se l'errore spinge ancora oltre il limite minimo
        if (error > 0.0f) {
            pid->integrator = tentative_integrator;
        }
    } else {
        // Entro i limiti lineari: aggiorna normalmente l'integratore
        pid->integrator = tentative_integrator;
    }

    // 7. Aggiorna stato per il prossimo intervallo di campionamento
    pid->prev_error = error;

    return output;
}
