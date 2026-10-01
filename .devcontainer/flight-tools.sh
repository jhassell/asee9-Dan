#!/usr/bin/env bash
# Flight-software toolchain for flightlib/ and the exercises in
# exercises/flight-software.md. Run by postCreate.sh when the Codespace (or its
# prebuild) is created. Safe to run again by hand to repair a missing tool:
#
#   bash .devcontainer/flight-tools.sh
#
# It takes a few minutes, mostly GNAT. Like postCreate.sh it never asks for or
# reads a key, and one failed step does not stop the others.
#
# VERSIONS. The Python packages are pinned exactly, like the rest of this
# container. The Debian packages are NOT pinned with pkg=version: Debian point
# releases replace old versions in the archive, and an exact pin would then fail
# outright. They are fixed in practice by the base image, which is pinned by
# digest to Debian 13 (trixie); in September 2026 that gave GCC 14, clang 19,
# cppcheck 2.17, GNAT 14, CBMC 6.6, valgrind 3.24, gdb 16.

APT_PKGS=(
  # C and C++: compilers, build tools, debugger
  build-essential clang gdb cmake ninja-build bear
  # static analysis and formatting
  cppcheck clang-tidy clang-format
  # dynamic analysis and coverage
  valgrind lcov
  # Ada (GNAT, with gprbuild) and Fortran
  gnat gprbuild gfortran
  # formal verification: the CBMC bounded model checker
  cbmc
  # code navigation
  universal-ctags cflow
  # WebAssembly: compile C to run in a browser (make -C flightlib web).
  # clang 19 targets wasm32-wasi with lld's wasm-ld, wasi-libc and the matching
  # compiler runtime; binaryen (wasm-opt) and wabt (wasm2wat) inspect and shrink.
  lld wasi-libc libclang-rt-19-dev-wasm32 binaryen wabt
)

#   numpy, scipy, sympy   numerics, signal processing, symbolic algebra
#   pytest, hypothesis    Python tests and property-based tests
#   gcovr                 coverage reports for C (make coverage)
#   jsbsim                the JSBSim flight dynamics model, with its aircraft
PIP_PKGS="numpy==2.5.3 scipy==1.18.1 sympy==1.14.0 pytest==9.1.1 hypothesis==6.168.3 gcovr==8.6 jsbsim==1.3.1"

echo "Installing the flight-software toolchain (a few minutes)..."
# DEBIAN_FRONTEND goes through `env`: sudo resets the environment, so an
# exported value would not reach apt-get.
# One unreachable or badly signed apt source makes `apt-get update` exit
# non-zero while the Debian lists still refresh, so carry on to the install.
sudo -n env DEBIAN_FRONTEND=noninteractive apt-get update -q \
  || echo "apt-get update reported a problem; trying the install anyway."
if sudo -n env DEBIAN_FRONTEND=noninteractive apt-get install -y -q --no-install-recommends "${APT_PKGS[@]}"; then
  echo "Flight-software apt packages installed."
else
  echo "Some apt packages did not install; the checks below say which."
fi
sudo -n apt-get clean >/dev/null 2>&1 || true

# shellcheck disable=SC2086
if pip install --user --quiet $PIP_PKGS; then
  echo "Flight-software Python packages installed."
else
  echo "Some flight-software Python packages did not install."
fi

# Report every tool by name, so a missing one is obvious in the creation log.
missing=()
for t in gcc clang cppcheck clang-tidy clang-format gdb valgrind cmake ninja \
         gnatmake gprbuild gfortran cbmc lcov bear ctags cflow wasm-ld wasm-opt wasm2wat; do
  command -v "$t" >/dev/null 2>&1 || missing+=("$t")
done
python3 -c 'import numpy, scipy, sympy, pytest, hypothesis, gcovr, jsbsim' 2>/dev/null \
  || missing+=("python:numpy/scipy/sympy/pytest/hypothesis/gcovr/jsbsim")
if [ "${#missing[@]}" -eq 0 ]; then
  echo "Flight-software toolchain complete."
else
  echo "Flight-software tools missing: ${missing[*]}"
fi
