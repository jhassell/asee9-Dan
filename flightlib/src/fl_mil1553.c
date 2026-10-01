/*
 * fl_mil1553.c - MIL-STD-1553B command word encode/decode.
 */
#include "fl_mil1553.h"

static uint8_t is_mode_subaddress(uint8_t sa)
{
    return ((sa == 0u) || (sa == 31u)) ? 1u : 0u;
}

fl_status_t fl_1553_cmd_encode(const fl_1553_cmd_t *cmd, uint16_t *word)
{
    uint16_t low5;

    if ((cmd == NULL) || (word == NULL)) {
        return FL_ERR_NULL;
    }
    if ((cmd->rt_address > 31u) || (cmd->transmit > 1u) ||
        (cmd->subaddress > 31u)) {
        return FL_ERR_RANGE;
    }

    if (is_mode_subaddress(cmd->subaddress) == 1u) {
        if (cmd->mode_code > 31u) {
            return FL_ERR_RANGE;
        }
        low5 = cmd->mode_code;
    } else {
        if ((cmd->word_count < 1u) || (cmd->word_count > 32u)) {
            return FL_ERR_RANGE;
        }
        /* 32 data words are sent as a word count field of 00000. */
        low5 = (uint16_t)(cmd->word_count & 0x1Fu);
    }

    *word = (uint16_t)(((uint16_t)cmd->rt_address << 11) |
                       ((uint16_t)cmd->transmit << 10) |
                       ((uint16_t)cmd->subaddress << 5) |
                       low5);
    return FL_OK;
}

fl_status_t fl_1553_cmd_decode(uint16_t word, fl_1553_cmd_t *cmd)
{
    uint8_t low5;

    if (cmd == NULL) {
        return FL_ERR_NULL;
    }
    cmd->rt_address = (uint8_t)((word >> 11) & 0x1Fu);
    cmd->transmit = (uint8_t)((word >> 10) & 0x1u);
    cmd->subaddress = (uint8_t)((word >> 5) & 0x1Fu);
    cmd->is_mode_code = is_mode_subaddress(cmd->subaddress);
    low5 = (uint8_t)(word & 0x1Fu);

    if (cmd->is_mode_code == 1u) {
        cmd->mode_code = low5;
        cmd->word_count = 0u;
    } else {
        cmd->mode_code = 0u;
        cmd->word_count = low5;
    }
    return FL_OK;
}

uint8_t fl_1553_parity(uint16_t word)
{
    uint16_t w = word;
    uint8_t ones = 0u;

    while (w != 0u) {
        ones = (uint8_t)(ones + (uint8_t)(w & 1u));
        w = (uint16_t)(w >> 1);
    }
    /* Odd parity: the parity bit is 1 when the data has an even count. */
    return (uint8_t)((ones & 1u) ^ 1u);
}
