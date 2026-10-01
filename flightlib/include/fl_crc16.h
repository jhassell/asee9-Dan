/*
 * fl_crc16.h - CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF, no reflection,
 * no final XOR). Check value for the ASCII bytes "123456789" is 0x29B1.
 *
 * Requirements: FL-CRC-001 .. FL-CRC-003 in docs/SPEC.md.
 */
#ifndef FL_CRC16_H
#define FL_CRC16_H

#include "fl_types.h"

#define FL_CRC16_INIT 0xFFFFu

/* CRC of len bytes, continuing from crc (pass FL_CRC16_INIT to start).
 * Feeding a message in pieces gives the same result as feeding it whole. */
uint16_t fl_crc16_update(uint16_t crc, const uint8_t *data, size_t len);

#endif /* FL_CRC16_H */
