/*
 * fl_arinc429.c - ARINC 429 word packing. See fl_arinc429.h for the layout.
 */
#include <math.h>
#include "fl_arinc429.h"

#define DATA_BITS    19u
#define DATA_MASK    0x7FFFFu        /* 19 bits */
#define SIGN_BIT     0x40000u        /* bit 29 within the data field */
#define MAG_BITS     18u

uint8_t fl_a429_label_reverse(uint8_t label)
{
    uint8_t out = 0u;
    unsigned i;

    for (i = 0u; i < 8u; i++) {
        out = (uint8_t)((uint8_t)(out << 1) | (uint8_t)((label >> i) & 1u));
    }
    return out;
}

uint32_t fl_a429_odd_parity_ok(uint32_t word)
{
    uint32_t ones = 0u;
    uint32_t w = word;

    while (w != 0u) {
        ones += w & 1u;
        w >>= 1;
    }
    return ones & 1u;
}

fl_status_t fl_a429_pack(const fl_a429_fields_t *f, uint32_t *word)
{
    uint32_t w;

    if ((f == NULL) || (word == NULL)) {
        return FL_ERR_NULL;
    }
    if ((f->sdi > 3u) || (f->ssm > 3u) || (f->data > DATA_MASK)) {
        return FL_ERR_RANGE;
    }

    w = (uint32_t)fl_a429_label_reverse(f->label);
    w |= ((uint32_t)f->sdi) << 8;
    w |= f->data << 10;
    w |= ((uint32_t)f->ssm) << 29;
    if (fl_a429_odd_parity_ok(w) == 0u) {
        w |= 0x80000000u;
    }
    *word = w;
    return FL_OK;
}

fl_status_t fl_a429_unpack(uint32_t word, fl_a429_fields_t *f)
{
    if (f == NULL) {
        return FL_ERR_NULL;
    }
    f->label = fl_a429_label_reverse((uint8_t)(word & 0xFFu));
    f->sdi = (uint8_t)((word >> 8) & 0x3u);
    f->data = (word >> 10) & DATA_MASK;
    f->ssm = (uint8_t)((word >> 29) & 0x3u);
    return (fl_a429_odd_parity_ok(word) == 1u) ? FL_OK : FL_ERR_PARITY;
}

fl_status_t fl_a429_bnr_encode(double value, double lsb, unsigned sig_bits,
                               uint32_t *data)
{
    double counts;
    long max_count;
    long n;
    uint32_t field;

    if (data == NULL) {
        return FL_ERR_NULL;
    }
    if ((sig_bits < 1u) || (sig_bits > MAG_BITS) || !(lsb > 0.0)) {
        return FL_ERR_RANGE;
    }

    counts = value / lsb;
    max_count = (1L << sig_bits) - 1L;
    if (!(counts <= (double)max_count) || !(counts >= (double)(-max_count - 1L))) {
        return FL_ERR_RANGE;
    }
    n = lround(counts);

    /* Two's complement in 19 bits, left-justified under the sign bit. */
    field = ((uint32_t)n << (MAG_BITS - sig_bits)) & DATA_MASK;
    *data = field;
    return FL_OK;
}

fl_status_t fl_a429_bnr_decode(uint32_t data, double lsb, unsigned sig_bits,
                               double *value)
{
    int32_t raw;
    int32_t scale;

    if (value == NULL) {
        return FL_ERR_NULL;
    }
    if ((sig_bits < 1u) || (sig_bits > MAG_BITS) || !(lsb > 0.0) ||
        (data > DATA_MASK)) {
        return FL_ERR_RANGE;
    }

    scale = (int32_t)(1L << (MAG_BITS - sig_bits));
    /* Unused low bits carry no information: ignore them. */
    raw = (int32_t)(data & ~((uint32_t)scale - 1u));
    if ((data & SIGN_BIT) != 0u) {
        raw -= (int32_t)(1L << DATA_BITS);
    }
    *value = ((double)(raw / scale)) * lsb;
    return FL_OK;
}
