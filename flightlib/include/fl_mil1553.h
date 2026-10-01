/*
 * fl_mil1553.h - MIL-STD-1553B command words.
 *
 * The 16 information bits of a command word, most significant first:
 *
 *   bits 15-11  remote terminal (RT) address, 0..31 (31 = broadcast)
 *   bit  10     transmit/receive: 1 = the RT transmits, 0 = it receives
 *   bits  9-5   subaddress 1..30, or 0/31 to mark a mode command
 *   bits  4-0   data word count, or the mode code for a mode command
 *
 * On the bus each word also carries a 3-bit-time sync and an odd parity bit;
 * fl_1553_parity() computes that bit for the 16 information bits.
 *
 * Requirements: FL-1553-001 .. FL-1553-004 in docs/SPEC.md.
 */
#ifndef FL_MIL1553_H
#define FL_MIL1553_H

#include "fl_types.h"

#define FL_1553_RT_BROADCAST  31u

typedef struct {
    uint8_t rt_address;     /* 0..31 */
    uint8_t transmit;       /* 1 = RT transmits, 0 = RT receives */
    uint8_t subaddress;     /* 0..31; 0 and 31 mean mode command */
    uint8_t is_mode_code;   /* set by decode: 1 when subaddress is 0 or 31 */
    uint8_t word_count;     /* data words, 1..32 (not a mode command) */
    uint8_t mode_code;      /* 0..31 (mode command only) */
} fl_1553_cmd_t;

/* Build a command word. For a data transfer, word_count is 1..32; for a mode
 * command (subaddress 0 or 31) mode_code is used and word_count ignored. */
fl_status_t fl_1553_cmd_encode(const fl_1553_cmd_t *cmd, uint16_t *word);

/* Split a command word into its fields. */
fl_status_t fl_1553_cmd_decode(uint16_t word, fl_1553_cmd_t *cmd);

/* The parity bit that makes the 17 bits (16 information + parity) odd. */
uint8_t fl_1553_parity(uint16_t word);

#endif /* FL_MIL1553_H */
