# flightlib - build, test and analyse.
#
#   make                build the library, the tests and sim_pitch
#   make test           run the unit tests
#   make analyze        cppcheck (with the MISRA C:2012 addon) and clang-tidy
#   make sanitize       rebuild the tests with AddressSanitizer + UBSan and run them
#   make valgrind       run the tests under valgrind's memcheck
#   make coverage       line and branch coverage of the tests (gcc + gcovr)
#   make determinism    run sim_pitch twice and at -O0 vs -O2; compare hashes
#   make legacy         build and run the legacy Fortran atmosphere (gfortran)
#   make web            compile the simulation to WebAssembly and install the web
#                       simulator in ../site/sim/ (public on port 8000)
#   make clean
#
# Override the compiler with CC=clang (or CC=gcc). Build output goes to build/.

CC      ?= gcc
OPT     ?= -O2
CSTD    := -std=c99
WARN    := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual \
           -Wstrict-prototypes -Wmissing-prototypes -Werror
CFLAGS  ?= $(OPT) -g
ALL_CFLAGS = $(CSTD) $(WARN) $(CFLAGS) -Iinclude
LDLIBS  := -lm

B       := build
SRCS    := $(wildcard src/*.c)
OBJS    := $(patsubst src/%.c,$(B)/%.o,$(SRCS))
LIB     := $(B)/libflight.a

.PHONY: all test analyze sanitize valgrind coverage determinism legacy web clean

all: $(LIB) $(B)/test_flightlib $(B)/sim_pitch

$(B):
	@mkdir -p $(B)

$(B)/%.o: src/%.c $(wildcard include/*.h) | $(B)
	$(CC) $(ALL_CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(B)/test_flightlib: tests/test_flightlib.c tests/fl_test.h $(LIB)
	$(CC) $(ALL_CFLAGS) tests/test_flightlib.c $(LIB) $(LDLIBS) -o $@

$(B)/sim_pitch: tools/sim_pitch.c $(LIB)
	$(CC) $(ALL_CFLAGS) tools/sim_pitch.c $(LIB) $(LDLIBS) -o $@

test: $(B)/test_flightlib
	./$(B)/test_flightlib

analyze:
	cppcheck --std=c99 --enable=warning,style,performance,portability \
	  --addon=misra --inline-suppr --error-exitcode=0 -Iinclude src/
	clang-tidy src/*.c -- $(CSTD) -Iinclude

sanitize: | $(B)
	$(CC) $(CSTD) $(WARN) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -Iinclude $(SRCS) tests/test_flightlib.c $(LDLIBS) -o $(B)/test_sanitize
	./$(B)/test_sanitize

valgrind: $(B)/test_flightlib
	valgrind --error-exitcode=1 --leak-check=full ./$(B)/test_flightlib

coverage: | $(B)
	rm -f $(B)/*.gcda $(B)/*.gcno
	gcc $(CSTD) $(WARN) -O0 -g --coverage -Iinclude $(SRCS) tests/test_flightlib.c \
	  $(LDLIBS) -o $(B)/test_coverage
	./$(B)/test_coverage
	python3 -m gcovr --root . --filter src/ --txt-metric branch --print-summary

determinism:
	$(MAKE) --no-print-directory B=$(B)/O0 OPT=-O0 $(B)/O0/sim_pitch
	$(MAKE) --no-print-directory B=$(B)/O2 OPT=-O2 $(B)/O2/sim_pitch
	@echo "-O0 run 1: $$(./$(B)/O0/sim_pitch 10 20 0.01 | tail -1)"
	@echo "-O0 run 2: $$(./$(B)/O0/sim_pitch 10 20 0.01 | tail -1)"
	@echo "-O2:       $$(./$(B)/O2/sim_pitch 10 20 0.01 | tail -1)"

legacy: | $(B)
	gfortran -std=legacy -O2 legacy/atmos77.f legacy/table77.f -o $(B)/table77
	./$(B)/table77

# WebAssembly: clang targets wasm32-wasi with Debian's wasi-libc (headers in
# /usr/include/wasm32-wasi, hence --sysroot=/usr). "reactor" = a library with
# exported functions and no main(). The module needs no imports at all.
WASM_CC      ?= clang
WASI_SYSROOT ?= /usr
SITE_DIR     ?= ../site
WASM_SRCS    := src/fl_atmos.c src/fl_pid.c src/fl_pitch.c src/fl_rk4.c web/sim_wasm.c

web: | $(B)
	@if $(WASM_CC) --target=wasm32-wasi --sysroot=$(WASI_SYSROOT) -mexec-model=reactor \
	    $(CSTD) -O2 -Wall -Wextra -Werror -Iinclude $(WASM_SRCS) -Wl,--strip-all \
	    -o $(B)/flightlib.wasm; then \
	  echo "Built $(B)/flightlib.wasm: the page will run the C engine."; \
	else \
	  rm -f $(B)/flightlib.wasm; \
	  echo "WebAssembly build failed; the page will run its JavaScript port instead."; \
	fi
	mkdir -p $(SITE_DIR)/sim
	cp web/index.html web/sim.js $(SITE_DIR)/sim/
	@if [ -f $(B)/flightlib.wasm ]; then cp $(B)/flightlib.wasm $(SITE_DIR)/sim/; \
	 else rm -f $(SITE_DIR)/sim/flightlib.wasm; fi
	@# A landing page, unless site/ already has a real one (site-public.sh's
	@# placeholder says "Your site will appear here" and is replaced).
	@if [ ! -f $(SITE_DIR)/index.html ] || grep -q "Your site will appear here" $(SITE_DIR)/index.html; then \
	  cp web/site-index.html $(SITE_DIR)/index.html; fi
	@echo "Installed: $(SITE_DIR)/sim/. Open it at your public site address, path /sim/"

clean:
	rm -rf $(B)
