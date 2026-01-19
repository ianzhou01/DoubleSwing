import createModule from "./doubleswing.js";

const canvas = document.getElementById("c");
const ctx = canvas.getContext("2d");
canvas.style.touchAction = "none";

// resizer for canvas
function resize() {
    const dpr = window.devicePixelRatio || 1;
    canvas.width = Math.floor(innerWidth * dpr);
    canvas.height = Math.floor(innerHeight * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
}
addEventListener("resize", resize);
resize();

// slaves (helpers)
function num(el) { return Number.parseFloat(el.value); }
function rad2deg(r) { return r * 180 / Math.PI; }

function normalizeAngle(a) {
    while (a < -Math.PI) a += 2 * Math.PI;
    while (a >  Math.PI) a -= 2 * Math.PI;
    return a;
}
function unwrapDelta(newTh, oldTh) {
    return normalizeAngle(newTh - oldTh);
}
function thetaFromMouse(dx, dy) {
    // theta measured from vertical, +y down => atan2(x, y)
    return normalizeAngle(Math.atan2(dx, dy));
}

function getCanvasPosFromClient(e) {
    const rect = canvas.getBoundingClientRect();
    return { x: e.clientX - rect.left, y: e.clientY - rect.top };
}

// scale based on CURRENT params
function pxPerM(params) {
    return Math.min(innerWidth, innerHeight) * 0.4 / (params.l1 + params.l2);
}

// WASM
const mod = await createModule();

// core
const ds_create = mod.cwrap("ds_create", "number",
    ["number","number","number","number","number","number","number","number","number","number"]);
const ds_destroy = mod.cwrap("ds_destroy", null, ["number"]);
const ds_step = mod.cwrap("ds_step", null, ["number","number"]);
const ds_step_drag_p1 = mod.cwrap("ds_step_drag_p1", null,
    ["number","number","number","number","number"]); // h,dt,th1,w1,a1
const ds_update_positions = mod.cwrap("ds_update_positions", null, ["number"]);

// cached positions
const ds_x1 = mod.cwrap("ds_x1", "number", []);
const ds_y1 = mod.cwrap("ds_y1", "number", []);
const ds_x2 = mod.cwrap("ds_x2", "number", []);
const ds_y2 = mod.cwrap("ds_y2", "number", []);

// state setters
const ds_set_th2 = mod.cwrap("ds_set_th2", null, ["number","number"]);
const ds_set_w2  = mod.cwrap("ds_set_w2",  null, ["number","number"]);
const ds_reset   = mod.cwrap("ds_reset",   null, ["number","number","number"]);

// param setters
const ds_set_l1      = mod.cwrap("ds_set_l1", null, ["number","number"]);
const ds_set_l2      = mod.cwrap("ds_set_l2", null, ["number","number"]);
const ds_set_m1      = mod.cwrap("ds_set_m1", null, ["number","number"]);
const ds_set_m2      = mod.cwrap("ds_set_m2", null, ["number","number"]);
const ds_set_damping = mod.cwrap("ds_set_damping", null, ["number","number"]);
const ds_set_g       = mod.cwrap("ds_set_g", null, ["number","number"]);

// getters (for readout)
const ds_th1 = mod.cwrap("ds_th1", "number", ["number"]);
const ds_w1  = mod.cwrap("ds_w1",  "number", ["number"]);
const ds_th2 = mod.cwrap("ds_th2", "number", ["number"]);
const ds_w2  = mod.cwrap("ds_w2",  "number", ["number"]);
const ds_energy = mod.cwrap("ds_energy", "number", ["number"]);

// ----- Le big bad source of truth >:) -----
const DEFAULT_PARAMS = {
    l1: 2.0,
    l2: 2.0,
    m1: 1.0,
    m2: 1.0,
    g: 9.80665,
    damping: 0.02,
};

const params = { ...DEFAULT_PARAMS };

// Engine
let h = ds_create(
    params.l1, params.l2, params.m1, params.m2, params.g, params.damping,
    0.0, 0.0,
    0.0, 0.0
);

// UI
const ui = {
    l1: document.getElementById("l1"),
    l2: document.getElementById("l2"),
    m1: document.getElementById("m1"),
    m2: document.getElementById("m2"),
    damping: document.getElementById("damping"),
    g: document.getElementById("g"),
    apply: document.getElementById("apply"),
    reset: document.getElementById("reset"),
    defaults: document.getElementById("defaults"),
    readout: document.getElementById("readout"),
};

// initialize inputs from params
function setInputsFromParams(p) {
    ui.l1.value = p.l1;
    ui.l2.value = p.l2;
    ui.m1.value = p.m1;
    ui.m2.value = p.m2;
    ui.damping.value = p.damping;
    ui.g.value = p.g;
}

function syncEngineParams() {
    ds_set_l1(h, params.l1);
    ds_set_l2(h, params.l2);
    ds_set_m1(h, params.m1);
    ds_set_m2(h, params.m2);
    ds_set_damping(h, params.damping);
    ds_set_g(h, params.g);
}

setInputsFromParams(params);

function resetFiltersAndTiming() {
    hasPrev = false;
    acc = 0.0;           // prevents burst of steps after UI changes
}

ui.apply.addEventListener("click", () => {
    if (dragging !== 0) return;

    params.l1 = num(ui.l1);
    params.l2 = num(ui.l2);
    params.m1 = num(ui.m1);
    params.m2 = num(ui.m2);
    params.damping = num(ui.damping);
    params.g = num(ui.g);

    syncEngineParams();

    resetFiltersAndTiming();
});

ui.defaults.addEventListener("click", () => {
    if (dragging !== 0) return;
    ds_reset(h, 0.0, 0.0);

    // reset parameter values
    Object.assign(params, DEFAULT_PARAMS);

    // update the UI inputs
    setInputsFromParams(params);

    syncEngineParams();

    resetFiltersAndTiming();

});

ui.reset.addEventListener("click", () => {
    ds_reset(h, 0.0, 0.0);
    resetFiltersAndTiming();
});

// Drag Filter
let dragging = 0; // 0 none, 1 bob1, 2 bob2
let prevTh = 0;
let hasPrev = false;
let filtOmega = 0;

const ALPHA = 0.20;
const OMEGA_MAX = 12.0;
const GRAB1 = 15;
const GRAB2 = 13;

function updateFilteredOmega(th, dt) {
    if (!hasPrev) {
        prevTh = th;
        filtOmega = 0;
        hasPrev = true;
        return 0;
    }
    const raw = unwrapDelta(th, prevTh) / dt;
    prevTh = th;

    const clamped = Math.max(-OMEGA_MAX, Math.min(OMEGA_MAX, raw));
    filtOmega = ALPHA * clamped + (1 - ALPHA) * filtOmega;
    return filtOmega;
}

// Pointer Events
let activePointerId = null;
let lastPointerPos = null;

canvas.addEventListener("pointerdown", (e) => {
    if (activePointerId !== null) return; // single pointer only
    activePointerId = e.pointerId;
    try { canvas.setPointerCapture(activePointerId); } catch {}

    lastPointerPos = getCanvasPosFromClient(e);

    // hit test bobs using current params + current positions
    const ppm = pxPerM(params);
    const ox = innerWidth * 0.5;
    const oy = innerHeight * 0.5;

    ds_update_positions(h);
    const x1 = ds_x1(), y1 = ds_y1();
    const x2 = ds_x2(), y2 = ds_y2();

    const p1 = { x: ox + x1 * ppm, y: oy + y1 * ppm };
    const p2 = { x: ox + x2 * ppm, y: oy + y2 * ppm };

    const d1 = Math.hypot(lastPointerPos.x - p1.x, lastPointerPos.y - p1.y);
    const d2 = Math.hypot(lastPointerPos.x - p2.x, lastPointerPos.y - p2.y);

    if (d2 <= GRAB2) dragging = 2;
    else if (d1 <= GRAB1) dragging = 1;
    else dragging = 0;

    hasPrev = false; // reset filter
    e.preventDefault();
}, { passive: false });

canvas.addEventListener("pointermove", (e) => {
    if (e.pointerId !== activePointerId) return;
    lastPointerPos = getCanvasPosFromClient(e);
    e.preventDefault();
}, { passive: false });

function endPointer(e) {
    if (e.pointerId !== activePointerId) return;
    dragging = 0;
    activePointerId = null;
    lastPointerPos = null;
    hasPrev = false;
    try { canvas.releasePointerCapture(e.pointerId); } catch {}
    e.preventDefault();
}
canvas.addEventListener("pointerup", endPointer, { passive: false });
canvas.addEventListener("pointercancel", endPointer, { passive: false });
canvas.addEventListener("pointerleave", endPointer, { passive: false });

// Keyboard reset
addEventListener("keydown", (e) => {
    if (e.key === "r" || e.key === "R") {
        ds_reset(h, 0.0, 0.0);
        resetFiltersAndTiming();
    }
});

// Readout
function updateReadout() {
    // positions already updated in frame()
    const th1v = ds_th1(h), w1v = ds_w1(h);
    const th2v = ds_th2(h), w2v = ds_w2(h);
    const x1 = ds_x1(), y1 = ds_y1(), x2 = ds_x2(), y2 = ds_y2();
    const E = ds_energy(h);

    ui.readout.textContent =
        `L1=${params.l1.toFixed(2)}  L2=${params.l2.toFixed(2)}  M1=${params.m1.toFixed(2)}  M2=${params.m2.toFixed(2)}
g=${params.g.toFixed(3)}  damping=${params.damping.toFixed(3)}

th1: ${rad2deg(th1v).toFixed(2)} deg   w1: ${w1v.toFixed(3)} rad/s
th2: ${rad2deg(th2v).toFixed(2)} deg   w2: ${w2v.toFixed(3)} rad/s

x1: ${x1.toFixed(3)}  y1: ${y1.toFixed(3)}
x2: ${x2.toFixed(3)}  y2: ${y2.toFixed(3)}

E: ${E.toFixed(4)}
dragging: ${dragging === 0 ? "none" : (dragging === 1 ? "bob1" : "bob2")}`;
}

// Animation
let last = performance.now();
let acc = 0.0;

const FIXED_DT = 1 / 240;
const MAX_FRAME = 1 / 15;
const MAX_STEPS = 8;

function frame(t) {
    let frameDt = (t - last) / 1000;
    last = t;

    frameDt = Math.min(frameDt, MAX_FRAME);
    acc += frameDt;

    const mp = lastPointerPos;

    const ppm = pxPerM(params);
    const ox = innerWidth * 0.5;
    const oy = innerHeight * 0.5;

    // needed for bob2 dragging
    let p1px = null;
    if (dragging === 2 && mp) {
        ds_update_positions(h);
        const x1 = ds_x1(), y1 = ds_y1();
        p1px = { x: ox + x1 * ppm, y: oy + y1 * ppm };
    }

    let steps = 0;
    while (acc >= FIXED_DT && steps < MAX_STEPS) {
        if (dragging === 1 && mp) {
            const dx = mp.x - ox;
            const dy = mp.y - oy;
            const th1 = thetaFromMouse(dx, dy);
            let w1 = updateFilteredOmega(th1, FIXED_DT);
            if (Math.abs(w1) < 0.05) w1 = 0.0;
            const a1 = 0.0;
            ds_step_drag_p1(h, FIXED_DT, th1, w1, a1);
        } else {
            if (dragging === 2 && mp && p1px) {
                const dx = mp.x - p1px.x;
                const dy = mp.y - p1px.y;
                const th2 = thetaFromMouse(dx, dy);
                const w2 = updateFilteredOmega(th2, FIXED_DT);
                ds_set_th2(h, th2);
                ds_set_w2(h, w2);
            }
            ds_step(h, FIXED_DT);
        }

        steps++;
        acc -= FIXED_DT;
    }

    ds_update_positions(h);
    const x1 = ds_x1(), y1 = ds_y1(), x2 = ds_x2(), y2 = ds_y2();

    draw(x1, y1, x2, y2);
    updateReadout();

    requestAnimationFrame(frame);
}

function draw(x1, y1, x2, y2) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const ppm = pxPerM(params);
    const ox = innerWidth * 0.5;
    const oy = innerHeight * 0.5;

    const p0 = { x: ox, y: oy };
    const p1 = { x: ox + x1 * ppm, y: oy + y1 * ppm };
    const p2 = { x: ox + x2 * ppm, y: oy + y2 * ppm };

    const snap = (v) => Math.round(v) + 0.5;
    p0.x = snap(p0.x); p0.y = snap(p0.y);
    p1.x = snap(p1.x); p1.y = snap(p1.y);
    p2.x = snap(p2.x); p2.y = snap(p2.y);

    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(p0.x, p0.y);
    ctx.lineTo(p1.x, p1.y);
    ctx.lineTo(p2.x, p2.y);
    ctx.stroke();

    const circle = (p, r) => {
        ctx.beginPath();
        ctx.arc(p.x, p.y, r, 0, Math.PI * 2);
        ctx.stroke();
    };

    circle(p0, 6);
    circle(p1, 10);
    circle(p2, 9);
}

requestAnimationFrame(frame);

// Cleanup
addEventListener("beforeunload", () => {
    try { ds_destroy(h); } catch {}
});
