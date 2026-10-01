# flightlib

A small, deterministic C99 library in the style of flight and simulator
software, written for this repository so an AI agent has realistic code to
work on. It is a teaching sample: not certified, not flight-tested, and the
pitch-model numbers are illustrative.

| Module | What it does | Requirements |
|---|---|---|
| `fl_atmos` | ISA / U.S. Standard Atmosphere 1976, 0-20 km | FL-ATM-* |
| `fl_arinc429` | ARINC 429 word pack/unpack, odd parity, BNR encode/decode | FL-A429-* |
| `fl_mil1553` | MIL-STD-1553B command words, mode codes, parity | FL-1553-* |
| `fl_crc16` | CRC-16/CCITT-FALSE | FL-CRC-* |
| `fl_pid` | fixed-step PID with output limits and anti-windup | FL-PID-* |
| `fl_rk4` | fixed-step RK4, no allocation | FL-RK4-* |
| `fl_pitch` | linear short-period pitch model | (used by `sim_pitch`) |

The requirements are in [`docs/SPEC.md`](docs/SPEC.md). Each test in
`tests/test_flightlib.c` names the requirements it covers.

## Layout

```
include/        public headers, one per module
src/            the library
tests/          unit tests and a 50-line test harness (fl_test.h)
tools/          sim_pitch.c (closed-loop pitch hold), jsbsim_hello.py
web/            the browser simulator: sim_wasm.c (exports), index.html, sim.js
legacy/         atmos77.f: a Fortran 77 standard atmosphere, an independent reference
docs/SPEC.md    the requirements
```

## Build and run

From this folder:

```
make                 # library, tests, sim_pitch (gcc; CC=clang also works)
make test            # unit tests
./build/sim_pitch 10 20 0.01 > build/step.csv     # 10 deg pitch step, 20 s, 100 Hz
make analyze         # cppcheck with the MISRA C:2012 addon, then clang-tidy
make sanitize        # tests under AddressSanitizer + UndefinedBehaviorSanitizer
make valgrind        # tests under valgrind memcheck
make coverage        # line and branch coverage (gcovr)
make determinism     # sim_pitch hash: two runs at -O0, one at -O2
make legacy          # the Fortran 77 atmosphere table (gfortran)
python3 tools/jsbsim_hello.py    # trim and fly JSBSim's Cessna 172 for 30 s
make web             # compile to WebAssembly, install the web simulator in ../site/sim/
```

Output goes to `build/`, which git ignores.

## The web simulator

`make web` compiles `fl_pid`, `fl_rk4`, `fl_pitch` and `fl_atmos` plus
`web/sim_wasm.c` to an 11 KB WebAssembly module with clang (target
`wasm32-wasi`, no imports needed), and copies it with `web/index.html` and
`web/sim.js` into `site/sim/`. The site on port 8000 is public, so the
simulator is live at your site address plus `/sim/`. Setup runs `make web` once
for you.

The page shows an attitude indicator, a strip chart and live controls (pitch
command, step buttons, gains, speed). It runs the **same C code** as the unit
tests: change `src/fl_pid.c`, run `make web`, refresh. If the WebAssembly
build is not available, the page falls back to a JavaScript port of the same
code (the badge at the top says which engine is running); the two were checked
to agree bit for bit.

To build a bigger simulator, start here: export more functions from
`sim_wasm.c`, draw more in `sim.js`, or precompute JSBSim runs into JSON for the
page to play back. Everything in `site/` is public while the Codespace runs.

## Conventions

- C99. No `malloc`, no recursion, no mutable global state.
- Every fallible function returns `fl_status_t`; results go through pointers.
- Fixed-size types (`uint32_t`, `uint16_t`) wherever bits matter.
- SI units unless a name says otherwise (`_deg`, `_ft`).
- Compiled with `-Wall -Wextra -Wpedantic -Wconversion -Werror`.

The MISRA addon in cppcheck checks rule numbers, not rule text: MISRA's rule
texts are copyrighted and not shipped with cppcheck. A finding prints as
"misra-c2012-15.5" and similar.
