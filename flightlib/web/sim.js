// sim.js - drives the pitch-hold simulation and draws it.
//
// Engine: flightlib.wasm (the C library compiled by `make web`) when present.
// If it is missing or fails to load, a JavaScript port of the same C code runs
// instead, and the badge at the top says so. The port follows src/fl_pid.c,
// src/fl_rk4.c and src/fl_pitch.c line for line, as they are written: change
// the C, and the port no longer matches until you change it too.

"use strict";

const DT = 0.01;            // s, fixed step (100 Hz)
const LIMIT_DEG = 20;       // elevator limit
const WINDOW_S = 20;        // strip chart width
const D2R = Math.PI / 180;
const R2D = 180 / Math.PI;

// ------------------------------------------------------------ engines

async function loadWasm() {
  const url = "flightlib.wasm";
  let result;
  try {
    result = await WebAssembly.instantiateStreaming(fetch(url), {});
  } catch (streamErr) {
    // Older servers send .wasm with the wrong MIME type: fall back.
    const resp = await fetch(url);
    if (!resp.ok) throw new Error("no " + url);
    result = await WebAssembly.instantiate(await resp.arrayBuffer(), {});
  }
  const e = result.instance.exports;
  if (e._initialize) e._initialize();
  return {
    name: "C (flightlib) via WebAssembly",
    kind: "c",
    reset: (kp, ki, kd) => e.sim_reset(kp, ki, kd, DT, LIMIT_DEG),
    setCommand: (c) => e.sim_set_command(c),
    step: (n) => e.sim_step(n),
    state: () => ({
      t: e.sim_time(), theta: e.sim_theta_deg(), q: e.sim_q_degs(),
      alpha: e.sim_alpha_deg(), de: e.sim_elevator_deg(),
    }),
  };
}

function jsEngine() {
  // fl_pitch_default
  const m = { za: -1.2, zde: -0.1, ma: -4.0, mq: -2.0, mde: -6.0, de: 0 };
  const pid = {};
  let x = [0, 0, 0], t = 0, cmd = 0, u = 0;

  function deriv(xs) {                         // fl_pitch_deriv
    return [
      m.za * xs[0] + xs[1] + m.zde * m.de,
      m.ma * xs[0] + m.mq * xs[1] + m.mde * m.de,
      xs[1],
    ];
  }
  function rk4() {                             // fl_rk4_step
    const h = 0.5 * DT;
    const k1 = deriv(x);
    const k2 = deriv(x.map((v, i) => v + h * k1[i]));
    const k3 = deriv(x.map((v, i) => v + h * k2[i]));
    const k4 = deriv(x.map((v, i) => v + DT * k3[i]));
    x = x.map((v, i) => v + (DT / 6) * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]));
  }
  function pidStep(command, meas) {            // fl_pid_step, as written
    const error = command - meas;
    if (!pid.primed) { pid.prev = meas; pid.primed = true; }
    const d = -(meas - pid.prev) / DT;
    pid.prev = meas;
    const stepI = pid.ki * error * DT;
    pid.integ += stepI;
    let out = pid.kp * error + pid.integ + pid.kd * d;
    if (out > LIMIT_DEG) { pid.integ -= stepI; out = LIMIT_DEG; }
    else if (out < -LIMIT_DEG) { out = -LIMIT_DEG; }
    return out;
  }
  return {
    name: "JavaScript port (run make web to use the C engine)",
    kind: "js",
    reset(kp, ki, kd) {
      Object.assign(pid, { kp, ki, kd, integ: 0, prev: 0, primed: false });
      x = [0, 0, 0]; t = 0; cmd = 0; u = 0; m.de = 0;
      return 0;
    },
    setCommand(c) { cmd = c; },
    step(n) {
      for (let k = 0; k < n; k++) {
        u = pidStep(cmd, x[2] * R2D);
        m.de = -u * D2R;
        rk4();
        t += DT;
      }
    },
    state: () => ({ t, theta: x[2] * R2D, q: x[1] * R2D, alpha: x[0] * R2D, de: m.de * R2D }),
  };
}

// ------------------------------------------------------------ app

const $ = (id) => document.getElementById(id);
const css = (name) => getComputedStyle(document.documentElement).getPropertyValue(name).trim();

let eng, paused = false, command = 0, hist = [], acc = 0, last = null;

function gains() {
  return [+$("kp").value, +$("ki").value, +$("kd").value];
}

function reset(newCommand) {
  eng.reset(...gains());
  command = newCommand;
  eng.setCommand(command);
  $("cmd").value = command;
  hist = [];
  acc = 0;
}

function frame(now) {
  if (last === null) last = now;
  const wall = Math.min((now - last) / 1000, 0.1);   // clamp after a hidden tab
  last = now;
  if (!paused) {
    acc += wall * +$("speed").value;
    const n = Math.floor(acc / DT);
    acc -= n * DT;
    for (let i = 0; i < n; i++) {
      eng.step(1);
      const s = eng.state();
      hist.push({ t: s.t, theta: s.theta, de: s.de, cmd: command });
    }
    while (hist.length && hist[hist.length - 1].t - hist[0].t > WINDOW_S) hist.shift();
  }
  draw();
  requestAnimationFrame(frame);
}

function draw() {
  const s = eng.state();
  $("r-t").textContent = s.t.toFixed(1) + " s";
  $("r-theta").textContent = s.theta.toFixed(2) + "°";
  $("r-cmd").textContent = command.toFixed(1) + "°";
  $("r-q").textContent = s.q.toFixed(2) + "°/s";
  $("r-alpha").textContent = s.alpha.toFixed(2) + "°";
  $("r-de").textContent = s.de.toFixed(2) + "°";
  $("cmd-val").textContent = command.toFixed(1) + "°";
  drawHorizon(s);
  drawChart();
}

function drawHorizon(s) {
  const c = $("horizon"), g = c.getContext("2d");
  const W = c.width, H = c.height, cx = W / 2, cy = H / 2;
  const pxPerDeg = H / 90;                    // +-45 degrees visible
  const off = s.theta * pxPerDeg;             // nose up: horizon moves down
  g.save();
  g.beginPath(); g.arc(cx, cy, W / 2 - 4, 0, 2 * Math.PI); g.clip();
  g.fillStyle = css("--sky"); g.fillRect(0, 0, W, cy + off);
  g.fillStyle = css("--ground"); g.fillRect(0, cy + off, W, H);
  g.strokeStyle = "#fff"; g.fillStyle = "#fff"; g.lineWidth = 2;
  g.font = "20px system-ui, sans-serif"; g.textAlign = "left"; g.textBaseline = "middle";
  for (let p = -60; p <= 60; p += 5) {
    const y = cy + off - p * pxPerDeg;
    const half = p === 0 ? W / 2 : (p % 10 === 0 ? 70 : 35);
    g.beginPath(); g.moveTo(cx - half, y); g.lineTo(cx + half, y); g.stroke();
    if (p !== 0 && p % 10 === 0) g.fillText(String(Math.abs(p)), cx + half + 8, y);
  }
  // Command bug
  const yc = cy + off - command * pxPerDeg;
  g.strokeStyle = css("--cmd"); g.lineWidth = 4;
  g.beginPath(); g.moveTo(cx - 130, yc); g.lineTo(cx - 95, yc); g.moveTo(cx + 95, yc); g.lineTo(cx + 130, yc); g.stroke();
  g.restore();
  // Fixed aircraft symbol
  g.strokeStyle = "#ffd400"; g.lineWidth = 6; g.lineCap = "round";
  g.beginPath(); g.moveTo(cx - 90, cy); g.lineTo(cx - 30, cy); g.lineTo(cx - 15, cy + 15);
  g.moveTo(cx + 90, cy); g.lineTo(cx + 30, cy); g.lineTo(cx + 15, cy + 15); g.stroke();
  g.beginPath(); g.arc(cx, cy, 5, 0, 2 * Math.PI); g.fillStyle = "#ffd400"; g.fill();
  // Elevator bar along the bottom
  const bw = W * 0.6, bx = cx - bw / 2, by = H - 40;
  g.fillStyle = "rgba(0,0,0,0.35)"; g.fillRect(bx, by, bw, 12);
  g.fillStyle = css("--elev");
  const frac = Math.max(-1, Math.min(1, s.de / LIMIT_DEG));
  g.fillRect(cx, by, frac * bw / 2, 12);
  g.beginPath(); g.arc(cx, cy, W / 2 - 4, 0, 2 * Math.PI);
  g.strokeStyle = css("--line"); g.lineWidth = 6; g.stroke();
}

function drawChart() {
  const c = $("chart"), g = c.getContext("2d");
  const W = c.width, H = c.height, padL = 60, padB = 40, padT = 16, padR = 16;
  const pw = W - padL - padR, ph = H - padT - padB;
  g.fillStyle = css("--panel"); g.fillRect(0, 0, W, H);
  const t1 = hist.length ? hist[hist.length - 1].t : 0;
  const t0 = Math.max(0, t1 - WINDOW_S);
  const X = (t) => padL + ((t - t0) / WINDOW_S) * pw;
  const Y = (v) => padT + ph / 2 - (v / 40) * (ph / 2);   // +-40 degrees
  g.strokeStyle = css("--line"); g.lineWidth = 1; g.fillStyle = css("--muted");
  g.font = "20px system-ui, sans-serif"; g.textAlign = "right"; g.textBaseline = "middle";
  for (let v = -40; v <= 40; v += 10) {
    g.beginPath(); g.moveTo(padL, Y(v)); g.lineTo(W - padR, Y(v)); g.stroke();
    g.fillText(v + "°", padL - 8, Y(v));
  }
  g.textAlign = "center"; g.textBaseline = "top";
  for (let t = Math.ceil(t0 / 2) * 2; t <= t0 + WINDOW_S; t += 2) {
    g.fillText(t + " s", X(t), H - padB + 8);
  }
  const series = (key, color, width) => {
    g.strokeStyle = color; g.lineWidth = width; g.beginPath();
    hist.forEach((h, i) => (i ? g.lineTo(X(h.t), Y(h[key])) : g.moveTo(X(h.t), Y(h[key]))));
    g.stroke();
  };
  series("cmd", css("--cmd"), 2);
  series("de", css("--elev"), 2);
  series("theta", css("--accent"), 4);
}

async function main() {
  try {
    eng = await loadWasm();
  } catch (err) {
    eng = jsEngine();
  }
  $("engine").textContent = eng.name;
  $("engine").className = "badge " + eng.kind;
  reset(0);

  $("cmd").addEventListener("input", (ev) => { command = +ev.target.value; eng.setCommand(command); });
  document.querySelectorAll("button[data-step]").forEach((b) =>
    b.addEventListener("click", () => reset(+b.dataset.step)));
  $("reset").addEventListener("click", () => reset(0));
  $("pause").addEventListener("click", (ev) => {
    paused = !paused; ev.target.textContent = paused ? "Resume" : "Pause";
  });
  ["kp", "ki", "kd"].forEach((id) => $(id).addEventListener("change", () => reset(command)));
  $("theme").addEventListener("click", () => {
    const root = document.documentElement;
    const dark = root.dataset.theme
      ? root.dataset.theme === "dark"
      : matchMedia("(prefers-color-scheme: dark)").matches;
    root.dataset.theme = dark ? "light" : "dark";
  });
  requestAnimationFrame(frame);
}

main();
