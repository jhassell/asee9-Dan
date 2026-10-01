/*
 * fl_pid.h - fixed-step PID controller with output limits and anti-windup.
 *
 * The derivative acts on the measurement, not the error, so a step in the
 * command does not kick the output.
 *
 * Requirements: FL-PID-001 .. FL-PID-004 in docs/SPEC.md.
 */
#ifndef FL_PID_H
#define FL_PID_H

#include "fl_types.h"

typedef struct {
    /* configuration */
    double kp;
    double ki;
    double kd;
    double dt;          /* fixed step, s, > 0 */
    double out_min;
    double out_max;     /* out_min < out_max */
    /* state */
    double integrator;
    double prev_meas;
    uint8_t primed;     /* 0 until the first step has run */
} fl_pid_t;

fl_status_t fl_pid_init(fl_pid_t *pid, double kp, double ki, double kd,
                        double dt, double out_min, double out_max);

/* One step. Writes the limited controller output to *out. */
fl_status_t fl_pid_step(fl_pid_t *pid, double command, double measurement,
                        double *out);

#endif /* FL_PID_H */
