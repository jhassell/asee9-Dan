/*
 * fl_pitch.h - linear short-period pitch model for control-law experiments.
 *
 * States: x[0] = angle of attack alpha (rad), x[1] = pitch rate q (rad/s),
 *         x[2] = pitch attitude theta (rad).
 * Input:  elevator deflection, rad; positive trailing edge down (nose down).
 *
 *   alpha' = Z_alpha * alpha + q + Z_de * de
 *   q'     = M_alpha * alpha + M_q * q + M_de * de
 *   theta' = q
 *
 * The coefficients are ILLUSTRATIVE, roughly a light single-engine aircraft
 * in cruise. They are not data for any real aircraft.
 */
#ifndef FL_PITCH_H
#define FL_PITCH_H

#include "fl_types.h"

#define FL_PITCH_STATES 3u

typedef struct {
    double z_alpha;     /* 1/s   */
    double z_de;        /* 1/s   */
    double m_alpha;     /* 1/s^2 */
    double m_q;         /* 1/s   */
    double m_de;        /* 1/s^2 */
    double de_rad;      /* current elevator input, held over a step */
} fl_pitch_model_t;

/* Fill in the illustrative default coefficients, elevator zero. */
void fl_pitch_default(fl_pitch_model_t *m);

/* Derivative function in the form fl_rk4_step() expects; ctx is a
 * const fl_pitch_model_t *. */
void fl_pitch_deriv(double t, const double *x, double *dxdt, const void *ctx);

#endif /* FL_PITCH_H */
