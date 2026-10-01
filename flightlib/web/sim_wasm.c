/*
 * sim_wasm.c - the browser's window into flightlib.
 *
 * Compiled with the library to WebAssembly by `make web`, so the web page runs
 * the same C code the unit tests run: fl_pid, fl_rk4, fl_pitch and fl_atmos.
 * It is a tool, not part of the library, so it may keep the simulation state
 * in one static struct (the library itself has no mutable global state).
 *
 * Every exported function takes and returns plain numbers, so JavaScript can
 * call it with no glue code. Units: degrees and seconds at this boundary.
 */
#include <math.h>
#include "fl_atmos.h"
#include "fl_pid.h"
#include "fl_pitch.h"
#include "fl_rk4.h"

#define EXPORT(name) __attribute__((export_name(#name)))

#define PI_D         3.14159265358979323846
#define DEG2RAD(d)   ((d) * (PI_D / 180.0))
#define RAD2DEG(r)   ((r) * (180.0 / PI_D))

static struct {
    fl_pitch_model_t model;
    fl_pid_t pid;
    double x[FL_PITCH_STATES];
    double t;
    double cmd_deg;
    double u_deg;
} sim;

/* Start over: aircraft trimmed level, controller reset. Returns 0 on success,
 * otherwise the fl_status_t from fl_pid_init. */
EXPORT(sim_reset)
int sim_reset(double kp, double ki, double kd, double dt, double limit_deg)
{
    fl_status_t s;

    fl_pitch_default(&sim.model);
    s = fl_pid_init(&sim.pid, kp, ki, kd, dt, -limit_deg, limit_deg);
    sim.x[0] = 0.0;
    sim.x[1] = 0.0;
    sim.x[2] = 0.0;
    sim.t = 0.0;
    sim.cmd_deg = 0.0;
    sim.u_deg = 0.0;
    return (int)s;
}

EXPORT(sim_set_command)
void sim_set_command(double theta_cmd_deg)
{
    sim.cmd_deg = theta_cmd_deg;
}

/* Advance n fixed steps: controller, then actuator, then RK4. */
EXPORT(sim_step)
void sim_step(int n)
{
    int k;

    for (k = 0; k < n; k++) {
        (void)fl_pid_step(&sim.pid, sim.cmd_deg, RAD2DEG(sim.x[2]), &sim.u_deg);
        sim.model.de_rad = DEG2RAD(-sim.u_deg);
        (void)fl_rk4_step(fl_pitch_deriv, &sim.model, sim.t, sim.pid.dt,
                          sim.x, FL_PITCH_STATES);
        sim.t += sim.pid.dt;
    }
}

EXPORT(sim_time)       double sim_time(void)       { return sim.t; }
EXPORT(sim_theta_deg)  double sim_theta_deg(void)  { return RAD2DEG(sim.x[2]); }
EXPORT(sim_q_degs)     double sim_q_degs(void)     { return RAD2DEG(sim.x[1]); }
EXPORT(sim_alpha_deg)  double sim_alpha_deg(void)  { return RAD2DEG(sim.x[0]); }
EXPORT(sim_elevator_deg) double sim_elevator_deg(void) { return RAD2DEG(sim.model.de_rad); }
EXPORT(sim_integrator) double sim_integrator(void) { return sim.pid.integrator; }

/* Standard atmosphere at a geopotential altitude in metres; NaN out of range. */
EXPORT(atmos_density)
double atmos_density(double h_m)
{
    fl_atmos_t a;
    return (fl_atmos_isa(h_m, &a) == FL_OK) ? a.density_kgm3 : NAN;
}

EXPORT(atmos_pressure)
double atmos_pressure(double h_m)
{
    fl_atmos_t a;
    return (fl_atmos_isa(h_m, &a) == FL_OK) ? a.pressure_pa : NAN;
}
