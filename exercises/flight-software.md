# Flight software with an agent

**Where you are:** setup printed **READY** and the agent opened in the terminal:
a bordered box with a cursor. Click once inside the terminal, type each line,
and press Enter once, at the end. Every step the agent takes shows as an
**Exec** card above its reply. **Check its evidence, not its summary.**

The code is [`flightlib/`](../flightlib/README.md): a small C99 library in the
style of avionics and simulator software (standard atmosphere, ARINC 429,
MIL-STD-1553B, CRC-16, PID, RK4, a pitch model), with requirements in
`flightlib/docs/SPEC.md`, unit tests, and a legacy Fortran routine.

**One rule before anything else.** What the agent reads is sent to OpenRouter
and the model's provider. **Nothing proprietary, nothing under NDA, nothing
ITAR or EAR controlled, ever.** Use `flightlib/` and open-source code only.

---

## Part 1: one goal, five moves (15 minutes)

### 1 Picture it

Don't type yet. You are handing this to a new engineer on their first day:

```
Build flightlib, run its tests, and tell me if it is ready to fly.
```

Write down what "ready" means to you, and what you would need to see before you
signed off on it.

### 2 Hand it over

Type the line above. Watch the Exec cards: which files it opens, which commands
it runs, and which it never looks at.

### 3 Compare

Hold its answer against what you wrote. Did it read `docs/SPEC.md`? Did it
treat "the tests pass" as "it works"? Who decided what "ready" meant?

### 4 Check

```
Which requirements in flightlib/docs/SPEC.md does no test fully cover?
```

"Fully" matters. A test can name a requirement and still check only part of
it. Open the test it points to and see for yourself.

### 5 Give it a rule

```
New rule: a requirement is met only if a test you ran proves it. Add the missing tests.
```

Now watch what fails. For each failure, ask yourself: is the code wrong, the
test wrong, or the requirement wrong? The agent will have an opinion. Make it
show you the evidence before you accept it, and do not let it "fix" a failing
test by weakening it.

---

## Part 2: things to try

Each is one typed line. They are independent; pick what interests you.

### An independent oracle

```
Compare fl_atmos against legacy/atmos77.f every 100 m from 0 to 20 km.
```

It has to build the Fortran (`make legacy` in `flightlib/`), write a harness for
the C, and line the two up. Two implementations that share no code are a classic
way to test numerical software. When they disagree, which one is right? Ask it
how it knows. The U.S. Standard Atmosphere 1976 is a public document.

### A formal proof, not a test

```
Use cbmc to prove fl_a429_pack always produces odd parity.
```

CBMC is a bounded model checker: it checks every possible input, not a sample.
The agent has to write a proof harness with `__CPROVER_assume` and read CBMC's
output. Then ask it what the proof does *not* cover.

### A mirror test

```
Run sim_pitch at +15 and -15 degrees. Should the responses mirror? Explain any difference.
```

The model is linear and the limits are symmetric (`FL-SIM-002`). See whether the
agent plots both, measures overshoot, and traces a difference back to a line of
code, or just describes the curves.

### Static analysis triage

```
Run make analyze in flightlib and sort every finding: real defect, MISRA deviation, or noise.
```

`cppcheck` runs with its MISRA C:2012 addon, then `clang-tidy`. Triage is the
tedious part of real code review. Spot-check three of its verdicts yourself.

### Bit-for-bit determinism

```
Is sim_pitch bit-identical between gcc and clang, at -O0 and -O2? Prove it with hashes.
```

`sim_pitch` prints an FNV-1a hash of every state at every step. Deterministic
replay matters in simulation. If the hashes differ, ask why; if they match, ask
what flags would break them (`-ffast-math`, `-march=native`, FMA contraction).

### Ada 2012 with contracts

```
Port fl_crc16 to Ada 2012 with Pre and Post contracts, and check it against the C on 100000 random inputs.
```

GNAT and gprbuild are installed. The agent writes the Ada, builds it, writes a
cross-check harness, and runs both. Read the contracts it chose: are they
meaningful, or just `True`?

### Real flight dynamics

```
Using flightlib/tools/jsbsim_hello.py as a start, fly the c172x through a 2-second elevator doublet and plot pitch rate.
```

JSBSim is an open-source six-degree-of-freedom flight dynamics model; the
Cessna 172 model ships with it. Then:

```
Compare that pitch response with sim_pitch. Where should they agree, and where not?
```

### A web simulator

Setup already compiled flightlib to WebAssembly and installed a live pitch-hold
simulator at your public site address plus `/sim/` (READY printed it). Open it,
press **Step +30°**, then **Step −30°**. Then:

```
Add an airspeed tape and an altitude readout to site/sim, driven by the C model.
```

```
Precompute a JSBSim c172x elevator doublet to JSON and add a page in site/ that plays it back beside sim_pitch.
```

```
Build site/atmos/: an interactive chart of fl_atmos from 0 to 20 km, computed by the WebAssembly module.
```

The agent edits `flightlib/web/`, runs `make -C flightlib web`, and checks with
`python3 .devcontainer/site-check.py`. Refresh the page to see each change.
Anything in `site/` is public while the Codespace runs.

### Coverage and sanitizers

```
Get flightlib to 100 percent branch coverage and show me the gcovr report.
```

```
Run make sanitize and make valgrind in flightlib. Are any findings real?
```

### Modernize legacy code

```
Rewrite legacy/atmos77.f as a Fortran 2018 module and prove the outputs match the original.
```

The part to watch: how does it define "match"? Exactly, or to a tolerance it
chose? Who should choose that?

### Open-source code you know

The agent can read any public repository. Clone one into `mine/` (it is
gitignored) and ask about it:

```
Shallow-clone github.com/JSBSim-Team/jsbsim into mine and explain how it integrates the equations of motion.
```

---

## What to notice

- **Tests passing is a claim about the tests.** Ask what they check.
- **The agent decides things you did not ask it to decide:** tolerances, which
  reference to trust, whether a failing test or the code is wrong. Name those
  decisions and make them yours.
- **Rules work.** A one-line rule ("never weaken a test to make it pass", "cite
  the command output for every claim") changes how it behaves for the rest of
  the conversation.
- **It is a coworker, not an oracle.** It reads, writes, runs and fixes faster
  than any of us. It also states things it never checked. Your job is the same
  as with any new engineer: specify, supervise, verify.

---

## The challenge

`flightlib` compiles warning-free under `-Wall -Wextra -Wpedantic -Wconversion
-Werror` and passes every one of its own tests. It still has deliberate
defects, of the kind that survive real reviews because the tests look complete.

Find them with the agent. For each one, make it:

1. name the requirement in `docs/SPEC.md` the code violates,
2. write a test that fails because of it, and run it,
3. fix the code, not the test, and show the test passing,
4. show that every other test still passes.

John has the answer key. Anything you find beyond it is real, and he would like
to hear about it.
