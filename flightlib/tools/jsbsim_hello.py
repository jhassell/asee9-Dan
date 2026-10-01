"""jsbsim_hello.py - a known-good starting point for JSBSim experiments.

JSBSim is the open-source flight dynamics model used by FlightGear and many
research simulators. The PyPI package bundles its aircraft models (c172x,
737, f16, ...) so nothing else needs downloading.

This script trims the Cessna 172 model (c172x) in level flight at 4000 ft and
100 knots, flies it hands-off for 30 s at the default 120 Hz, prints a few
samples and writes every frame to build/jsbsim_c172.csv.

    python3 tools/jsbsim_hello.py

Run from the flightlib/ folder. Ask the agent to build on it: an elevator
doublet, a different aircraft, a plot, a comparison with sim_pitch.
"""
import csv
import os
import sys

import jsbsim

OUT = os.path.join("build", "jsbsim_c172.csv")
PROPS = [
    ("t_s", "simulation/sim-time-sec"),
    ("alt_ft", "position/h-sl-ft"),
    ("kcas", "velocities/vc-kts"),
    ("theta_deg", "attitude/theta-deg"),
    ("alpha_deg", "aero/alpha-deg"),
    ("q_degs", "velocities/q-rad_sec"),
    ("elevator_deg", "fcs/elevator-pos-deg"),
    ("pitch_trim", "fcs/pitch-trim-cmd-norm"),
    ("throttle_cmd", "fcs/throttle-cmd-norm"),
]


def main():
    fdm = jsbsim.FGFDMExec(None)          # None = the bundled aircraft data
    fdm.set_debug_level(0)
    # The c172x model also tells JSBSim to log to JSBout172B.csv. Keep that
    # file in build/ with everything else, not in the source folder.
    os.makedirs("build", exist_ok=True)
    fdm.set_output_path(os.path.abspath("build"))
    if not fdm.load_model("c172x"):
        sys.exit("Could not load the c172x model.")

    fdm["ic/h-sl-ft"] = 4000.0
    fdm["ic/vc-kts"] = 100.0
    fdm["ic/psi-true-deg"] = 0.0
    fdm["ic/gamma-deg"] = 0.0
    fdm["propulsion/set-running"] = -1    # all engines running
    fdm.run_ic()
    fdm["simulation/do_simple_trim"] = 1  # 1 = full longitudinal trim

    os.makedirs("build", exist_ok=True)
    with open(OUT, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow([name for name, _ in PROPS])
        while fdm["simulation/sim-time-sec"] < 30.0:
            row = [fdm[p] for _, p in PROPS]
            row[5] = row[5] * 57.29577951308232    # rad/s -> deg/s
            w.writerow(row)
            fdm.run()

    print(f"dt = {fdm.get_delta_t():.6f} s")
    with open(OUT) as f:
        rows = list(csv.DictReader(f))
    for r in rows[:: len(rows) // 5]:
        print(f"t={float(r['t_s']):5.1f}s  alt={float(r['alt_ft']):7.1f} ft  "
              f"KCAS={float(r['kcas']):6.2f}  theta={float(r['theta_deg']):6.3f} deg  "
              f"elevator={float(r['elevator_deg']):+.3f} deg  trim={float(r['pitch_trim']):+.4f}")
    print(f"Wrote {len(rows)} frames to {OUT}")


if __name__ == "__main__":
    main()
