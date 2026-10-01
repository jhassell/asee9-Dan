/*
 * fl_arinc429.h - ARINC 429 32-bit word packing.
 *
 * Bit numbering follows the ARINC 429 specification: bit 1 is the least
 * significant bit of the uint32_t, bit 32 the most significant.
 *
 *   bits  1-8   label (octal), stored bit-reversed: the label is sent most
 *               significant bit first, so bit 1 holds the label's MSB
 *   bits  9-10  SDI (source/destination identifier)
 *   bits 11-29  data field; for BNR, bit 29 is the sign and bits 11-28 the
 *               two's-complement magnitude, most significant bit at bit 28
 *   bits 30-31  SSM (sign/status matrix)
 *   bit  32     parity, set so the word has an odd number of ones
 *
 * Requirements: FL-A429-001 .. FL-A429-005 in docs/SPEC.md.
 */
#ifndef FL_ARINC429_H
#define FL_ARINC429_H

#include "fl_types.h"

/* BNR sign/status matrix values (bits 30-31). */
#define FL_A429_SSM_FAILURE      0u
#define FL_A429_SSM_NO_DATA      1u
#define FL_A429_SSM_TEST         2u
#define FL_A429_SSM_NORMAL       3u

typedef struct {
    uint8_t label;      /* label as written in octal, e.g. 0203 for 203 */
    uint8_t sdi;        /* 0..3 */
    uint8_t ssm;        /* 0..3 */
    uint32_t data;      /* raw 19-bit data field, bits 11-29 */
} fl_a429_fields_t;

/* Reverse the order of the 8 bits of a label. */
uint8_t fl_a429_label_reverse(uint8_t label);

/* 1 if the word as given has an odd number of one bits, else 0. */
uint32_t fl_a429_odd_parity_ok(uint32_t word);

/* Pack fields into a word and set bit 32 for odd parity. */
fl_status_t fl_a429_pack(const fl_a429_fields_t *f, uint32_t *word);

/* Unpack a word. Returns FL_ERR_PARITY (fields still filled) if parity fails. */
fl_status_t fl_a429_unpack(uint32_t word, fl_a429_fields_t *f);

/*
 * BNR data field for a signed value.
 *   lsb       weight of the least significant used bit, e.g. 1.0 ft
 *   sig_bits  number of magnitude bits used, 1..18, left-justified below
 *             the sign bit (unused low bits are zero)
 * Values beyond the representable range return FL_ERR_RANGE.
 */
fl_status_t fl_a429_bnr_encode(double value, double lsb, unsigned sig_bits,
                               uint32_t *data);
fl_status_t fl_a429_bnr_decode(uint32_t data, double lsb, unsigned sig_bits,
                               double *value);

#endif /* FL_ARINC429_H */
