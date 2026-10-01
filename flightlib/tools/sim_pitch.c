/*
 * sim_pitch.c - closed-loop pitch-attitude hold: PID -> elevator -> RK4 pitch
 * model, fixed step, fully deterministic.
 *
 *   build/sim_pitch [command_deg] [duration_s] [dt_s]
 *
 * Defaults: 10 degrees, 20 s, 0.01 s. Prints CSV on stdout:
 *   t_s,theta_deg,q_degs,alpha_deg,elevator_deg
 * and a last line "# fnv1a64 <hex>": a hash of the exact bits of every state
 * and output at every step. Two runs agree bit for bit if and only if their
 * hashes agree.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fl_pid.h"
#include "fl_rk4.h"
#include "fl_pitch.h"

#define PI_D        3.14159265358979323846
#define DEG2RAD(d)  ((d) * (PI_D / 180.0))
#define RAD2DEG(r)  ((r) * (180.0 / PI_D))
#define DE_LIMIT_DEG 20.0

static uint64_t fnv1a64(uint64_t h, const void *p, size_t n)
{
    const unsigned char *b = (const unsigned char *)p;
    size_t i;

    for (i = 0u; i < n; i++) {
        h ^= (uint64_t)b[i];
        h *= 1099511628211ULL;
    }
    return h;
}

int main(int argc, char **argv)
{
    double cmd_deg = (argc > 1) ? atof(argv[1]) : 10.0;
    double duration = (argc > 2) ? atof(argv[2]) : 20.0;
    double dt = (argc > 3) ? atof(argv[3]) : 0.01;
    double x[FL_PITCH_STATES] = {0.0, 0.0, 0.0};
    fl_pitch_model_t model;
    fl_pid_t pid;
    uint64_t hash = 14695981039346656037ULL;
    long steps;
    long k;

    if (!(dt > 0.0) || !(duration > 0.0)) {
        fprintf(stderr, "usage: sim_pitch [command_deg] [duration_s] [dt_s]\n");
        return 2;
    }
    fl_pitch_default(&model);
    /* The controller works in degrees of pitch and commands nose-up
     * elevator as a positive number; the model's elevator is positive
     * nose-down, so the sign flips at the actuator. */
    if (fl_pid_init(&pid, 4.0, 2.0, 1.2, dt, -DE_LIMIT_DEG, DE_LIMIT_DEG) != FL_OK) {
        fprintf(stderr, "bad controller settings\n");
        return 2;
    }

    steps = (long)(duration / dt + 0.5);
    printf("t_s,theta_deg,q_degs,alpha_deg,elevator_deg\n");
    for (k = 0; k <= steps; k++) {
        double t = (double)k * dt;
        double u_deg;

        (void)fl_pid_step(&pid, cmd_deg, RAD2DEG(x[2]), &u_deg);
        model.de_rad = DEG2RAD(-u_deg);
        printf("%.3f,%.6f,%.6f,%.6f,%.6f\n", t, RAD2DEG(x[2]), RAD2DEG(x[1]),
               RAD2DEG(x[0]), RAD2DEG(model.de_rad));
        hash = fnv1a64(hash, x, sizeof x);
        hash = fnv1a64(hash, &model.de_rad, sizeof model.de_rad);
        (void)fl_rk4_step(fl_pitch_deriv, &model, t, dt, x, FL_PITCH_STATES);
    }
    printf("# fnv1a64 %016llx\n", (unsigned long long)hash);
    return 0;
}
