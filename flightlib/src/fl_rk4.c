/*
 * fl_rk4.c - classic RK4, fixed step.
 */
#include "fl_rk4.h"

fl_status_t fl_rk4_step(fl_deriv_fn f, const void *ctx, double t, double dt,
                        double *x, size_t n)
{
    double k1[FL_RK4_MAX_STATES];
    double k2[FL_RK4_MAX_STATES];
    double k3[FL_RK4_MAX_STATES];
    double k4[FL_RK4_MAX_STATES];
    double tmp[FL_RK4_MAX_STATES];
    double half;
    size_t i;

    if ((f == NULL) || (x == NULL)) {
        return FL_ERR_NULL;
    }
    if ((n == 0u) || (n > FL_RK4_MAX_STATES) || !(dt > 0.0)) {
        return FL_ERR_RANGE;
    }
    half = 0.5 * dt;

    f(t, x, k1, ctx);
    for (i = 0u; i < n; i++) {
        tmp[i] = x[i] + (half * k1[i]);
    }
    f(t + half, tmp, k2, ctx);
    for (i = 0u; i < n; i++) {
        tmp[i] = x[i] + (half * k2[i]);
    }
    f(t + half, tmp, k3, ctx);
    for (i = 0u; i < n; i++) {
        tmp[i] = x[i] + (dt * k3[i]);
    }
    f(t + dt, tmp, k4, ctx);
    for (i = 0u; i < n; i++) {
        x[i] += (dt / 6.0) * (k1[i] + (2.0 * k2[i]) + (2.0 * k3[i]) + k4[i]);
    }
    return FL_OK;
}
