# flightlib software requirements

Requirement IDs are cited in `tests/test_flightlib.c` ("Covers: FL-...").
"Shall" statements are the requirements. The notes under them are not.

This is a teaching sample written in the style of a requirements document for
airborne or simulator software. It is not certified to anything, and the
numbers in the pitch model are illustrative, not data for a real aircraft.

## General

**FL-GEN-001.** The library shall be ISO C99, shall compile with no warnings
under the flags in the `Makefile`, and shall use no dynamic memory allocation,
no recursion and no mutable global or static state.

**FL-GEN-002.** Every function that can fail shall return an `fl_status_t` and
shall not write through a NULL pointer argument.

## Standard atmosphere (`fl_atmos`)

**FL-ATM-001.** At 0 m, `fl_atmos_isa` shall return 288.15 K, 101325 Pa,
1.2250 kg/m³ (±0.01%) and 340.29 m/s (±0.01%).

**FL-ATM-002.** For every geopotential altitude from 0 to 20000 m inclusive,
temperature, pressure, density and speed of sound shall match the U.S.
Standard Atmosphere, 1976 within 0.1%.

**FL-ATM-003.** An altitude below 0 m or above 20000 m shall return
`FL_ERR_RANGE`.

**FL-ATM-004.** A NULL output pointer shall return `FL_ERR_NULL`.

## ARINC 429 (`fl_arinc429`)

**FL-A429-001.** `fl_a429_pack` shall place the label bit-reversed in bits 1-8,
the SDI in bits 9-10, the data field in bits 11-29 and the SSM in bits 30-31,
and `fl_a429_unpack` shall recover each field exactly.

**FL-A429-002.** `fl_a429_pack` shall set bit 32 so that the word has odd
parity.

**FL-A429-003.** `fl_a429_unpack` shall return `FL_ERR_PARITY` for a word with
even parity, and shall still fill in the fields.

**FL-A429-004.** For any value representable with the given LSB and number of
significant bits, positive or negative, BNR encode followed by decode shall
return the value within half an LSB.

**FL-A429-005.** BNR encode shall return `FL_ERR_RANGE` for a value outside the
representable range, and for an LSB that is not positive or a significant-bit
count outside 1-18.

## MIL-STD-1553B (`fl_mil1553`)

**FL-1553-001.** `fl_1553_cmd_encode` shall pack the RT address (bits 15-11),
T/R (bit 10), subaddress (bits 9-5) and word count or mode code (bits 4-0),
and shall return `FL_ERR_RANGE` for an RT address or subaddress above 31, a T/R
above 1, or a word count outside 1-32.

**FL-1553-002.** For every RT address, T/R value, data subaddress (1-30) and
word count (1-32), encode followed by decode shall return the original fields.

**FL-1553-003.** A subaddress of 0 or 31 shall mark a mode command: decode shall
set `is_mode_code` and return the low five bits as `mode_code`.

**FL-1553-004.** `fl_1553_parity` shall return the bit that gives the 16
information bits plus the parity bit an odd number of ones.

**FL-1553-005.** A word count field of 00000 in a data (non-mode) command shall
decode as 32 data words, as MIL-STD-1553B specifies.

## CRC-16 (`fl_crc16`)

**FL-CRC-001.** `fl_crc16_update` shall implement CRC-16/CCITT-FALSE: for the
nine ASCII bytes `123456789` starting from `FL_CRC16_INIT`, it shall return
0x29B1.

**FL-CRC-002.** Feeding a message in any number of pieces shall give the same
CRC as feeding it whole.

**FL-CRC-003.** A NULL data pointer shall return the input CRC unchanged.

## PID controller (`fl_pid`)

**FL-PID-001.** `fl_pid_init` shall reject a step size that is not positive and
limits where `out_min` is not below `out_max`.

**FL-PID-002.** The output of `fl_pid_step` shall always lie within
[`out_min`, `out_max`].

**FL-PID-003.** Anti-windup: while the output is held at either limit, the
integrator shall not move further toward that limit.

**FL-PID-004.** The proportional term shall be `kp × error`, and the derivative
term shall act on the measurement, so that a step in the command produces no
derivative kick.

## Integrator (`fl_rk4`)

**FL-RK4-001.** `fl_rk4_step` shall be the classic fourth-order Runge-Kutta
method: for dx/dt = -x from x = 1 with ten steps of 0.1 s, the result shall be
within 1e-6 of e⁻¹.

**FL-RK4-002.** A state count of 0 or above `FL_RK4_MAX_STATES`, a step that is
not positive, or a NULL function or state pointer shall return an error and
leave the state unchanged.

## Simulation (`tools/sim_pitch.c`)

**FL-SIM-001.** `sim_pitch` shall be deterministic: the same arguments shall
produce the same `fnv1a64` hash on every run, at every optimisation level.

**FL-SIM-002.** The pitch model is linear and the controller limits are
symmetric (±20°), so the closed-loop response to a command of -c shall be the
mirror image of the response to +c.
