/*
 * test_flightlib.c - unit tests for flightlib.
 *
 * Each test names the requirement(s) in docs/SPEC.md it covers.
 */
#include <string.h>
#include "fl_test.h"
#include "fl_atmos.h"
#include "fl_arinc429.h"
#include "fl_mil1553.h"
#include "fl_crc16.h"
#include "fl_pid.h"
#include "fl_rk4.h"

/* ---------------------------------------------------------------- atmos */

/* Covers: FL-ATM-001 */
static void test_atmos_sea_level(void)
{
    fl_atmos_t a;
    CHECK(fl_atmos_isa(0.0, &a) == FL_OK);
    CHECK_NEAR(a.temperature_k, 288.15, 1e-9);
    CHECK_NEAR(a.pressure_pa, 101325.0, 1e-6);
    CHECK_NEAR(a.density_kgm3, 1.225, 1e-4);
    CHECK_NEAR(a.sound_speed_ms, 340.294, 1e-3);
}

/* Covers: FL-ATM-002 (US Standard Atmosphere 1976, geopotential altitude) */
static void test_atmos_reference_points(void)
{
    fl_atmos_t a;
    CHECK(fl_atmos_isa(5000.0, &a) == FL_OK);
    CHECK_NEAR(a.temperature_k, 255.65, 1e-9);
    CHECK_NEAR(a.pressure_pa, 54019.9, 0.001 * 54019.9);
    CHECK_NEAR(a.density_kgm3, 0.736116, 0.001 * 0.736116);

    CHECK(fl_atmos_isa(11000.0, &a) == FL_OK);
    CHECK_NEAR(a.temperature_k, 216.65, 1e-9);
    CHECK_NEAR(a.pressure_pa, 22632.06, 0.001 * 22632.06);
    CHECK_NEAR(a.density_kgm3, 0.363918, 0.001 * 0.363918);
}

/* Covers: FL-ATM-003, FL-ATM-004 */
static void test_atmos_rejects_bad_input(void)
{
    fl_atmos_t a;
    CHECK(fl_atmos_isa(-1.0, &a) == FL_ERR_RANGE);
    CHECK(fl_atmos_isa(20000.5, &a) == FL_ERR_RANGE);
    CHECK(fl_atmos_isa(1000.0, NULL) == FL_ERR_NULL);
}

/* ------------------------------------------------------------- ARINC 429 */

/* Covers: FL-A429-001 */
static void test_a429_label_reverse(void)
{
    unsigned l;
    CHECK(fl_a429_label_reverse(0203u) == 0xC1u);   /* 1000 0011 -> 1100 0001 */
    CHECK(fl_a429_label_reverse(0001u) == 0x80u);
    for (l = 0u; l < 256u; l++) {
        CHECK(fl_a429_label_reverse(fl_a429_label_reverse((uint8_t)l)) == l);
    }
}

/* Covers: FL-A429-001, FL-A429-002 */
static void test_a429_pack_unpack(void)
{
    fl_a429_fields_t in = {0205u, 2u, FL_A429_SSM_NORMAL, 0x12345u};
    fl_a429_fields_t out;
    uint32_t w = 0u;

    CHECK(fl_a429_pack(&in, &w) == FL_OK);
    CHECK(fl_a429_odd_parity_ok(w) == 1u);
    CHECK(fl_a429_unpack(w, &out) == FL_OK);
    CHECK(out.label == in.label);
    CHECK(out.sdi == in.sdi);
    CHECK(out.ssm == in.ssm);
    CHECK(out.data == in.data);

    in.data = 0u;
    in.sdi = 0u;
    CHECK(fl_a429_pack(&in, &w) == FL_OK);
    CHECK(fl_a429_odd_parity_ok(w) == 1u);
}

/* Covers: FL-A429-003 */
static void test_a429_parity_error(void)
{
    fl_a429_fields_t in = {0203u, 0u, FL_A429_SSM_NORMAL, 1000u};
    fl_a429_fields_t out;
    uint32_t w = 0u;

    CHECK(fl_a429_pack(&in, &w) == FL_OK);
    CHECK(fl_a429_unpack(w ^ 0x00000400u, &out) == FL_ERR_PARITY);  /* one data bit */
    CHECK(fl_a429_unpack(w ^ 0x80000000u, &out) == FL_ERR_PARITY);  /* the parity bit */
}

/* Covers: FL-A429-004 */
static void test_a429_bnr_round_trip(void)
{
    uint32_t d = 0u;
    double v = 0.0;

    /* Pressure altitude style: 1 ft LSB, 17 significant bits. */
    CHECK(fl_a429_bnr_encode(1000.0, 1.0, 17u, &d) == FL_OK);
    CHECK(fl_a429_bnr_decode(d, 1.0, 17u, &v) == FL_OK);
    CHECK_NEAR(v, 1000.0, 1e-9);

    CHECK(fl_a429_bnr_encode(-500.0, 1.0, 17u, &d) == FL_OK);
    CHECK(fl_a429_bnr_decode(d, 1.0, 17u, &v) == FL_OK);
    CHECK_NEAR(v, -500.0, 1e-9);
    CHECK((d & 0x40000u) != 0u);                  /* sign bit set */

    /* Angle style: 180/2^12 degree LSB, 12 significant bits. */
    CHECK(fl_a429_bnr_encode(45.0, 180.0 / 4096.0, 12u, &d) == FL_OK);
    CHECK(fl_a429_bnr_decode(d, 180.0 / 4096.0, 12u, &v) == FL_OK);
    CHECK_NEAR(v, 45.0, 1e-9);
}

/* ------------------------------------------------------------ MIL-STD-1553 */

/* Covers: FL-1553-001 */
static void test_1553_encode_known_word(void)
{
    fl_1553_cmd_t c;
    uint16_t w = 0u;

    memset(&c, 0, sizeof c);
    c.rt_address = 5u;
    c.transmit = 1u;
    c.subaddress = 3u;
    c.word_count = 4u;
    CHECK(fl_1553_cmd_encode(&c, &w) == FL_OK);
    CHECK(w == 0x2C64u);    /* 00101 1 00011 00100 */

    c.rt_address = 32u;
    CHECK(fl_1553_cmd_encode(&c, &w) == FL_ERR_RANGE);
    c.rt_address = 5u;
    c.word_count = 0u;
    CHECK(fl_1553_cmd_encode(&c, &w) == FL_ERR_RANGE);
}

/* Covers: FL-1553-002 */
static void test_1553_round_trip(void)
{
    fl_1553_cmd_t c;
    fl_1553_cmd_t d;
    uint16_t w = 0u;
    unsigned wc;

    memset(&c, 0, sizeof c);
    c.rt_address = 12u;
    c.transmit = 0u;
    c.subaddress = 7u;
    for (wc = 1u; wc <= 31u; wc++) {
        c.word_count = (uint8_t)wc;
        CHECK(fl_1553_cmd_encode(&c, &w) == FL_OK);
        CHECK(fl_1553_cmd_decode(w, &d) == FL_OK);
        CHECK(d.rt_address == 12u);
        CHECK(d.transmit == 0u);
        CHECK(d.subaddress == 7u);
        CHECK(d.is_mode_code == 0u);
        CHECK(d.word_count == wc);
    }
}

/* Covers: FL-1553-003 */
static void test_1553_mode_code(void)
{
    fl_1553_cmd_t c;
    fl_1553_cmd_t d;
    uint16_t w = 0u;

    memset(&c, 0, sizeof c);
    c.rt_address = 1u;
    c.transmit = 1u;
    c.subaddress = 31u;
    c.mode_code = 2u;           /* transmit status word */
    CHECK(fl_1553_cmd_encode(&c, &w) == FL_OK);
    CHECK(fl_1553_cmd_decode(w, &d) == FL_OK);
    CHECK(d.is_mode_code == 1u);
    CHECK(d.mode_code == 2u);
}

/* Covers: FL-1553-004 */
static void test_1553_parity(void)
{
    CHECK(fl_1553_parity(0x0000u) == 1u);
    CHECK(fl_1553_parity(0x0001u) == 0u);
    CHECK(fl_1553_parity(0x2C64u) == 1u);      /* six ones */
    CHECK(fl_1553_parity(0xFFFFu) == 1u);
}

/* ------------------------------------------------------------------- CRC */

/* Covers: FL-CRC-001 */
static void test_crc16_check_value(void)
{
    const uint8_t msg[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    CHECK(fl_crc16_update(FL_CRC16_INIT, msg, sizeof msg) == 0x29B1u);
}

/* Covers: FL-CRC-002 */
static void test_crc16_incremental(void)
{
    const uint8_t msg[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint16_t c = fl_crc16_update(FL_CRC16_INIT, msg, 4u);
    c = fl_crc16_update(c, &msg[4], 5u);
    CHECK(c == 0x29B1u);
}

/* ------------------------------------------------------------------- PID */

/* Covers: FL-PID-001 */
static void test_pid_init_checks(void)
{
    fl_pid_t p;
    CHECK(fl_pid_init(&p, 1.0, 0.0, 0.0, 0.0, -1.0, 1.0) == FL_ERR_RANGE);
    CHECK(fl_pid_init(&p, 1.0, 0.0, 0.0, 0.01, 1.0, 1.0) == FL_ERR_RANGE);
    CHECK(fl_pid_init(NULL, 1.0, 0.0, 0.0, 0.01, -1.0, 1.0) == FL_ERR_NULL);
    CHECK(fl_pid_init(&p, 1.0, 0.0, 0.0, 0.01, -1.0, 1.0) == FL_OK);
}

/* Covers: FL-PID-002 */
static void test_pid_output_limits(void)
{
    fl_pid_t p;
    double u = 0.0;

    CHECK(fl_pid_init(&p, 10.0, 0.0, 0.0, 0.01, -2.0, 3.0) == FL_OK);
    CHECK(fl_pid_step(&p, 100.0, 0.0, &u) == FL_OK);
    CHECK_NEAR(u, 3.0, 0.0);
    CHECK(fl_pid_step(&p, -100.0, 0.0, &u) == FL_OK);
    CHECK_NEAR(u, -2.0, 0.0);
}

/* Covers: FL-PID-003 */
static void test_pid_anti_windup(void)
{
    fl_pid_t p;
    double u = 0.0;
    int k;

    /* Hold a large positive error for 5 s: the output stays at its limit,
     * and the integrator must not wind up behind it. */
    CHECK(fl_pid_init(&p, 1.0, 1.0, 0.0, 0.01, -1.0, 1.0) == FL_OK);
    for (k = 0; k < 500; k++) {
        CHECK(fl_pid_step(&p, 10.0, 0.0, &u) == FL_OK);
    }
    CHECK_NEAR(u, 1.0, 0.0);
    CHECK(p.integrator <= 1.0);
}

/* Covers: FL-PID-004 */
static void test_pid_proportional_and_derivative(void)
{
    fl_pid_t p;
    double u = 0.0;

    CHECK(fl_pid_init(&p, 2.0, 0.0, 0.5, 0.1, -100.0, 100.0) == FL_OK);
    CHECK(fl_pid_step(&p, 1.0, 0.0, &u) == FL_OK);
    CHECK_NEAR(u, 2.0, 1e-12);           /* first step: no derivative kick */
    CHECK(fl_pid_step(&p, 5.0, 0.0, &u) == FL_OK);
    CHECK_NEAR(u, 10.0, 1e-12);          /* command step: still no kick */
    CHECK(fl_pid_step(&p, 5.0, 1.0, &u) == FL_OK);
    CHECK_NEAR(u, 8.0 - 5.0, 1e-12);     /* measurement moved 1 in 0.1 s */
}

/* ------------------------------------------------------------------- RK4 */

static void decay(double t, const double *x, double *dxdt, const void *ctx)
{
    (void)t;
    (void)ctx;
    dxdt[0] = -x[0];
}

/* Covers: FL-RK4-001 */
static void test_rk4_exponential_decay(void)
{
    double x[1] = {1.0};
    int k;

    for (k = 0; k < 10; k++) {
        CHECK(fl_rk4_step(decay, NULL, 0.1 * k, 0.1, x, 1u) == FL_OK);
    }
    CHECK_NEAR(x[0], exp(-1.0), 1e-6);
}

/* Covers: FL-RK4-002 */
static void test_rk4_rejects_bad_input(void)
{
    double x[1] = {1.0};
    CHECK(fl_rk4_step(decay, NULL, 0.0, 0.1, x, 0u) == FL_ERR_RANGE);
    CHECK(fl_rk4_step(decay, NULL, 0.0, 0.1, x, FL_RK4_MAX_STATES + 1u) == FL_ERR_RANGE);
    CHECK(fl_rk4_step(decay, NULL, 0.0, 0.0, x, 1u) == FL_ERR_RANGE);
    CHECK(fl_rk4_step(NULL, NULL, 0.0, 0.1, x, 1u) == FL_ERR_NULL);
}

int main(void)
{
    RUN(test_atmos_sea_level);
    RUN(test_atmos_reference_points);
    RUN(test_atmos_rejects_bad_input);
    RUN(test_a429_label_reverse);
    RUN(test_a429_pack_unpack);
    RUN(test_a429_parity_error);
    RUN(test_a429_bnr_round_trip);
    RUN(test_1553_encode_known_word);
    RUN(test_1553_round_trip);
    RUN(test_1553_mode_code);
    RUN(test_1553_parity);
    RUN(test_crc16_check_value);
    RUN(test_crc16_incremental);
    RUN(test_pid_init_checks);
    RUN(test_pid_output_limits);
    RUN(test_pid_anti_windup);
    RUN(test_pid_proportional_and_derivative);
    RUN(test_rk4_exponential_decay);
    RUN(test_rk4_rejects_bad_input);

    printf("\n%d checks, %d failed\n", fl_test_checks, fl_test_failures);
    return (fl_test_failures == 0) ? 0 : 1;
}
