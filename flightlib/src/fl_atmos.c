/*
 * fl_atmos.c - ISA 1976 standard atmosphere, 0 to 20 km geopotential.
 */
#include <math.h>
#include "fl_atmos.h"

/* US Standard Atmosphere 1976 constants. */
#define G0       9.80665       /* standard gravity, m/s^2 */
#define R_AIR    287.05287     /* specific gas constant of dry air, J/(kg K) */
#define GAMMA    1.4           /* ratio of specific heats */
#define T0       288.15        /* sea-level temperature, K */
#define P0       101325.0      /* sea-level pressure, Pa */
#define LAPSE    (-0.0065)     /* troposphere lapse rate, K/m */
#define H_TROPO  11000.0       /* tropopause, m */
#define T_TROPO  216.65        /* tropopause temperature, K */
#define P_TROPO  22632.0640    /* tropopause pressure, Pa */

fl_status_t fl_atmos_isa(double h_m, fl_atmos_t *out)
{
    double t;
    double p;

    if (out == NULL) {
        return FL_ERR_NULL;
    }
    if ((h_m < FL_ATM_H_MIN_M) || (h_m > FL_ATM_H_MAX_M)) {
        return FL_ERR_RANGE;
    }

    if (h_m <= H_TROPO) {
        /* Troposphere: linear temperature, pressure from the hydrostatic
         * equation with a constant lapse rate. */
        t = T0 + (LAPSE * h_m);
        p = P0 * pow(t / T0, -G0 / (LAPSE * R_AIR));
    } else {
        /* Lower stratosphere: isothermal layer. */
        t = T_TROPO;
        p = P_TROPO * exp((-G0 * (h_m - H_TROPO)) / (R_AIR * T0));
    }

    out->temperature_k = t;
    out->pressure_pa = p;
    out->density_kgm3 = p / (R_AIR * t);
    out->sound_speed_ms = sqrt(GAMMA * R_AIR * t);
    return FL_OK;
}
