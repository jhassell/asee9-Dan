/*
 * fl_types.h - common types and status codes for flightlib.
 *
 * flightlib conventions (see flightlib/README.md):
 *   - C99, no dynamic memory, no recursion, no global mutable state.
 *   - Every function that can fail returns fl_status_t and writes its
 *     results through pointer arguments.
 *   - Units are SI unless a name says otherwise (_deg, _ft, _kt).
 */
#ifndef FL_TYPES_H
#define FL_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    FL_OK = 0,
    FL_ERR_NULL = 1,     /* a required pointer argument was NULL */
    FL_ERR_RANGE = 2,    /* an input was outside its documented range */
    FL_ERR_PARITY = 3    /* a received word failed its parity check */
} fl_status_t;

#endif /* FL_TYPES_H */
