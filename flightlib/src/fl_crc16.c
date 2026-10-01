/*
 * fl_crc16.c - CRC-16/CCITT-FALSE, bitwise (no table: smaller, easier to
 * review, fast enough for message-sized buffers).
 */
#include "fl_crc16.h"

#define CRC16_POLY 0x1021u

uint16_t fl_crc16_update(uint16_t crc, const uint8_t *data, size_t len)
{
    uint16_t c = crc;
    size_t i;
    unsigned bit;

    if (data == NULL) {
        return c;
    }
    for (i = 0u; i < len; i++) {
        c = (uint16_t)(c ^ (uint16_t)((uint16_t)data[i] << 8));
        for (bit = 0u; bit < 8u; bit++) {
            if ((c & 0x8000u) != 0u) {
                c = (uint16_t)((uint16_t)(c << 1) ^ CRC16_POLY);
            } else {
                c = (uint16_t)(c << 1);
            }
        }
    }
    return c;
}
