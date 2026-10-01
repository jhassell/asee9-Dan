/*
 * fl_rk4.h - fixed-step fourth-order Runge-Kutta integrator.
 *
 * No allocation: the state is at most FL_RK4_MAX_STATES doubles and all
 * scratch space lives on the stack.
 *
 * Requirements: FL-RK4-001 .. FL-RK4-002 in docs/SPEC.md.
 */
#ifndef FL_RK4_H
#define FL_RK4_H

#include "fl_types.h"

#define FL_RK4_MAX_STATES 8u

/* dx/dt = f(t, x, u). Writes n derivatives to dxdt. ctx is passed through. */
typedef void (*fl_deriv_fn)(double t, const double *x, double *dxdt,
                            const void *ctx);

/* Advance x (n states) from t to t + dt in place. */
fl_status_t fl_rk4_step(fl_deriv_fn f, const void *ctx, double t, double dt,
                        double *x, size_t n);

#endif /* FL_RK4_H */
