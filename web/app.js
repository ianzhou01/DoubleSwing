import createModule from "./doubleswing.js";

const canvas = document.getElementById("c");
const ctx = canvas.getContext("2d");

function resize() {
    const dpr = window.devicePixelRatio || 1;
    canvas.width = Math.floor(innerWidth * dpr);
    canvas.height = Math.floor(innerHeight * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
}
addEventListener("resize", resize);
resize();

// Load WASM, bind C funcs
const mod = await createModule();

const ds_create = mod.cwrap("ds_create", "number",
    ["number","number","number","number","number","number","number","number","number","number"]);
const ds_destroy = mod.cwrap("ds_destroy", null, ["number"]);
const ds_step = mod.cwrap("ds_step", null, ["number","number"]);
const ds_step_drag_p1 = mod.cwrap("ds_step_drag_p1", null, ["number", "number", "number","number"]);
const ds_update_positions = mod.cwrap("ds_update_positions", null, ["number"]);

const ds_x1 = mod.cwrap("ds_x1", "number", []);
const ds_y1 = mod.cwrap("ds_y1", "number", []);
const ds_x2 = mod.cwrap("ds_x2", "number", []);
const ds_y2 = mod.cwrap("ds_y2", "number", []);

const ds_set_th1 = mod.cwrap("ds_set_th1", null, ["number","number"]);
const ds_set_th2 = mod.cwrap("ds_set_th2", null, ["number","number"]);
const ds_set_w1  = mod.cwrap("ds_set_w1",  null, ["number","number"]);
const ds_set_w2  = mod.cwrap("ds_set_w2",  null, ["number","number"]);
const ds_reset   = mod.cwrap("ds_reset",   null, ["number","number","number"]);

// Create engine
const l1 = 2.0, l2 = 2.0;
const m1 = 1.0, m2 = 1.0;
const g = 9.80665;
const damping = 0.0;

let h = ds_create(
    l1, l2, m1, m2, g, damping,
    0.0, 0.0,   // th1, w1
    0.0, 0.0    // th2, w2
);

let dragging = 0; // 0 none, 1 bob1, 2 bob2
let prevTh = 0;
let hasPrev = false;
let filtOmega = 0;

const ALPHA = 0.20;      // low-pass filter strength
const OMEGA_MAX = 12.0;  // clamp rad/s (tune)
const GRAB1 = 14;        // px, match your draw radius
const GRAB2 = 12;

function normalizeAngle(a) {
    while (a < -Math.PI) a += 2*Math.PI;
    while (a >  Math.PI) a -= 2*Math.PI;
    return a;
}

function unwrapDelta(newTh, oldTh) {
    let d = normalizeAngle(newTh - oldTh);
    return d;
}

// angle from vertical (+y down), consistent with your engine:
// bob_positions uses x = L*sin(th), y = L*cos(th)
// so th = atan2(x, y) where x=dx, y=dy
function thetaFromMouse(dx, dy) {
    return normalizeAngle(Math.atan2(dx, dy));
}

function updateFilteredOmega(th, dt) {
    if (!hasPrev) {
        prevTh = th;
        filtOmega = 0;
        hasPrev = true;
        return 0;
    }
    const raw = unwrapDelta(th, prevTh) / dt;
    prevTh = th;

    // clamp then low-pass
    const clamped = Math.max(-OMEGA_MAX, Math.min(OMEGA_MAX, raw));
    filtOmega = ALPHA * clamped + (1 - ALPHA) * filtOmega;
    return filtOmega;
}

function getMouse(e) {
    const rect = canvas.getBoundingClientRect();
    return {
        x: e.clientX - rect.left,
        y: e.clientY - rect.top
    };
}

canvas.addEventListener("mousedown", (e) => {
    const m = getMouse(e);

    // compute current pixel positions (same math as draw)
    const pxPerM = Math.min(innerWidth, innerHeight) * 0.4 / (l1 + l2);
    const ox = innerWidth * 0.5;
    const oy = innerHeight * 0.5;

    ds_update_positions(h);
    const x1 = ds_x1(), y1 = ds_y1(), x2 = ds_x2(), y2 = ds_y2();

    const p1 = { x: ox + x1 * pxPerM, y: oy + y1 * pxPerM };
    const p2 = { x: ox + x2 * pxPerM, y: oy + y2 * pxPerM };

    const d1 = Math.hypot(m.x - p1.x, m.y - p1.y);
    const d2 = Math.hypot(m.x - p2.x, m.y - p2.y);

    if (d2 <= GRAB2) dragging = 2;
    else if (d1 <= GRAB1) dragging = 1;
    else dragging = 0;

    // reset filter so first derivative isn't garbage
    hasPrev = false;

    e.preventDefault();
});

addEventListener("mouseup", () => {
    dragging = 0;
    hasPrev = false; // fresh start next drag
});

addEventListener("keydown", (e) => {
    if (e.key === "r" || e.key === "R") {
        ds_reset(h, 0.0, 0.0); // match your initial angles
        hasPrev = false;
    }
});

addEventListener("mousemove", (e) => {
    const rect = canvas.getBoundingClientRect();
    window._lastMousePos = { x: e.clientX - rect.left, y: e.clientY - rect.top };
});



// Anim loop
let last = performance.now();
let acc = 0.0;

const FIXED_DT = 1 / 240;     // physics tick
const MAX_FRAME = 1 / 15;     // cap huge tab-out jumps
const MAX_STEPS = 8;          // prevent spiral-of-death

function frame(t) {
    let frameDt = (t - last) / 1000;
    last = t;

    frameDt = Math.min(frameDt, MAX_FRAME);
    acc += frameDt;

    // Sample mouse + compute any geometry ONCE per frame
    const mp = window._lastMousePos;

    const pxPerM = Math.min(innerWidth, innerHeight) * 0.4 / (l1 + l2);
    const ox = innerWidth * 0.5;
    const oy = innerHeight * 0.5;

    // Only needed if dragging bob2 (angle about bob1)
    let p1 = null;
    if (dragging === 2 && mp) {
        ds_update_positions(h);
        const x1 = ds_x1(), y1 = ds_y1();
        p1 = { x: ox + x1 * pxPerM, y: oy + y1 * pxPerM };
    }

    let steps = 0;
    while (acc >= FIXED_DT && steps < MAX_STEPS) {

        if (dragging === 1 && mp) {
            const dx = mp.x - ox;
            const dy = mp.y - oy;
            const th1 = thetaFromMouse(dx, dy);
            let w1  = updateFilteredOmega(th1, FIXED_DT);
            if (Math.abs(w1) < 0.05) w1 = 0.0;
            const a1  = 0.0; // start with no tangential angular accel
            ds_step_drag_p1(h, FIXED_DT, th1, w1, a1);

        } else {
            // bob2
            if (dragging === 2 && mp && p1) {
                const dx = mp.x - p1.x;
                const dy = mp.y - p1.y;
                const th2 = thetaFromMouse(dx, dy);
                const w2  = updateFilteredOmega(th2, FIXED_DT);
                ds_set_th2(h, th2);
                ds_set_w2(h, w2);
            }

            // Normal coupled step
            ds_step(h, FIXED_DT);
        }

        steps++;
        acc -= FIXED_DT;
    }

    // Now update positions once for rendering
    ds_update_positions(h);
    const x1 = ds_x1(), y1 = ds_y1(), x2 = ds_x2(), y2 = ds_y2();
    draw(x1, y1, x2, y2);

    requestAnimationFrame(frame);
}

function draw(x1, y1, x2, y2) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const pxPerM = Math.min(innerWidth, innerHeight) * 0.4 / (l1 + l2);
    const ox = innerWidth * 0.5;
    const oy = innerHeight * 0.5;

    const p0 = { x: ox, y: oy };
    const p1 = { x: ox + x1 * pxPerM, y: oy + y1 * pxPerM };
    const p2 = { x: ox + x2 * pxPerM, y: oy + y2 * pxPerM };
    const snap = (v) => Math.round(v) + 0.5;
    p0.x = snap(p0.x); p0.y = snap(p0.y);
    p1.x = snap(p1.x); p1.y = snap(p1.y);
    p2.x = snap(p2.x); p2.y = snap(p2.y);

    // rods
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(p0.x, p0.y);
    ctx.lineTo(p1.x, p1.y);
    ctx.lineTo(p2.x, p2.y);
    ctx.stroke();

    // bobs
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
