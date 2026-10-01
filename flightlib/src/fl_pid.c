/*
 * fl_pid.c - fixed-step PID with conditional-integration anti-windup.
 */
#include "fl_pid.h"

fl_status_t fl_pid_init(fl_pid_t *pid, double kp, double ki, double kd,
                        double dt, double out_min, double out_max)
{
    if (pid == NULL) {
        return FL_ERR_NULL;
    }
    if (!(dt > 0.0) || !(out_min < out_max)) {
        return FL_ERR_RANGE;
    }
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->dt = dt;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integrator = 0.0;
    pid->prev_meas = 0.0;
    pid->primed = 0u;
    return FL_OK;
}

fl_status_t fl_pid_step(fl_pid_t *pid, double command, double measurement,
                        double *out)
{
    double error;
    double deriv;
    double u;
    double step_i;

    if ((pid == NULL) || (out == NULL)) {
        return FL_ERR_NULL;
    }

    error = command - measurement;
    if (pid->primed == 0u) {
        pid->prev_meas = measurement;
        pid->primed = 1u;
    }
    deriv = -(measurement - pid->prev_meas) / pid->dt;
    pid->prev_meas = measurement;

    step_i = pid->ki * error * pid->dt;
    pid->integrator += step_i;
    u = (pid->kp * error) + pid->integrator + (pid->kd * deriv);

    /* Anti-windup: while the output is limited, stop integrating. */
    if (u > pid->out_max) {
        pid->integrator -= step_i;
        u = pid->out_max;
    } else if (u < pid->out_min) {
        u = pid->out_min;
    } else {
        /* within limits */
    }

    *out = u;
    return FL_OK;
}
