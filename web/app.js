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

// Load WASM
const mod = await createModule();

// Bind C functions
const ds_create = mod.cwrap("ds_create", "number",
    ["number","number","number","number","number","number","number","number","number","number"]);
const ds_destroy = mod.cwrap("ds_destroy", null, ["number"]);
const ds_step = mod.cwrap("ds_step", null, ["number","number"]);
const ds_update_positions = mod.cwrap("ds_update_positions", null, ["number"]);

const ds_x1 = mod.cwrap("ds_x1", "number", []);
const ds_y1 = mod.cwrap("ds_y1", "number", []);
const ds_x2 = mod.cwrap("ds_x2", "number", []);
const ds_y2 = mod.cwrap("ds_y2", "number", []);

// Create engine
const l1 = 2.0, l2 = 2.0;
const m1 = 1.0, m2 = 1.0;
const g = 9.80665;
const damping = 0.0;

let h = ds_create(
    l1, l2, m1, m2, g, damping,
    0.3, 0.0,   // th1, w1
    0.6, 0.0    // th2, w2
);

// Anim loop
let last = performance.now();

function frame(t) {
    const dt = Math.min((t - last) / 1000, 1 / 30);
    last = t;

    ds_step(h, dt);
    ds_update_positions(h);

    const x1 = ds_x1();
    const y1 = ds_y1();
    const x2 = ds_x2();
    const y2 = ds_y2();

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

    // rods
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
