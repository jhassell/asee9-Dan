/*
 * fl_atmos.h - International Standard Atmosphere (ISA, US Standard
 * Atmosphere 1976), troposphere and lower stratosphere.
 *
 * Requirements: FL-ATM-001 .. FL-ATM-004 in docs/SPEC.md.
 */
#ifndef FL_ATMOS_H
#define FL_ATMOS_H

#include "fl_types.h"

#define FL_ATM_H_MIN_M      0.0      /* lowest supported geopotential altitude */
#define FL_ATM_H_MAX_M  20000.0      /* highest supported geopotential altitude */

typedef struct {
    double temperature_k;   /* static air temperature, K */
    double pressure_pa;     /* static pressure, Pa */
    double density_kgm3;    /* air density, kg/m^3 */
    double sound_speed_ms;  /* speed of sound, m/s */
} fl_atmos_t;

/* Standard-day properties at geopotential altitude h_m (metres). */
fl_status_t fl_atmos_isa(double h_m, fl_atmos_t *out);

#endif /* FL_ATMOS_H */
