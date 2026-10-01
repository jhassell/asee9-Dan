/*
 * fl_pitch.c - linear short-period pitch model. See fl_pitch.h.
 */
#include "fl_pitch.h"

void fl_pitch_default(fl_pitch_model_t *m)
{
    if (m == NULL) {
        return;
    }
    m->z_alpha = -1.2;
    m->z_de = -0.1;
    m->m_alpha = -4.0;
    m->m_q = -2.0;
    m->m_de = -6.0;
    m->de_rad = 0.0;
}

void fl_pitch_deriv(double t, const double *x, double *dxdt, const void *ctx)
{
    const fl_pitch_model_t *m = (const fl_pitch_model_t *)ctx;

    (void)t;    /* time-invariant model */
    dxdt[0] = (m->z_alpha * x[0]) + x[1] + (m->z_de * m->de_rad);
    dxdt[1] = (m->m_alpha * x[0]) + (m->m_q * x[1]) + (m->m_de * m->de_rad);
    dxdt[2] = x[1];
}
