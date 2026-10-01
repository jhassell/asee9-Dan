/*
 * fl_test.h - a minimal test harness: no framework to install or pin.
 *
 * Each check prints nothing on success and one line on failure. main()
 * returns the number of failed checks (0 = all passed).
 * The comment above each test names the requirement(s) in docs/SPEC.md it
 * covers, e.g. "Covers: FL-ATM-001".
 */
#ifndef FL_TEST_H
#define FL_TEST_H

#include <stdio.h>
#include <math.h>

static int fl_test_checks = 0;
static int fl_test_failures = 0;

#define CHECK(cond) do { \
    fl_test_checks++; \
    if (!(cond)) { \
        fl_test_failures++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_NEAR(a, b, tol) do { \
    double fl_a_ = (double)(a); \
    double fl_b_ = (double)(b); \
    fl_test_checks++; \
    if (!(fabs(fl_a_ - fl_b_) <= (double)(tol))) { \
        fl_test_failures++; \
        printf("FAIL %s:%d: %s = %.9g, expected %.9g (tol %g)\n", \
               __FILE__, __LINE__, #a, fl_a_, fl_b_, (double)(tol)); \
    } \
} while (0)

#define RUN(test) do { \
    int fl_before_ = fl_test_failures; \
    test(); \
    printf("%-36s %s\n", #test, (fl_test_failures == fl_before_) ? "ok" : "FAILED"); \
} while (0)

#endif /* FL_TEST_H */
