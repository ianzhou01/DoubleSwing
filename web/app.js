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

const mod = await createModule();

const ds_create = mod.cwrap("ds_create", "number",
    ["number","number","number","number","number","number","number","number","number","number"]);
const ds_destroy = mod.cwrap("ds_destroy", null, ["number"]);
const ds_step = mod.cwrap("ds_step", null, ["number","number"]);
const ds_bob_positions = mod.cwrap("ds_bob_positions", null,
    ["number","number","number","number","number"]);

const l1=2.0, l2=2.0, m1=1.0, m2=1.0, g=9.80665, damping=0.0;
let th1=0.2, w1=0.0, th2=0.4, w2=0.0;

const h = ds_create(l1,l2,m1,m2,g,damping, th1,w1,th2,w2);

const buf = mod._malloc(8*4);
const x1p = buf, y1p = buf+8, x2p = buf+16, y2p = buf+24;

let last = performance.now();
function frame(t){
    const dt = Math.min((t-last)/1000, 1/30);
    last = t;

    ds_step(h, dt);
    ds_bob_positions(h, x1p, y1p, x2p, y2p);

    const x1 = mod.getValue(x1p, "double");
    const y1 = mod.getValue(y1p, "double");
    const x2 = mod.getValue(x2p, "double");
    const y2 = mod.getValue(y2p, "double");

    ctx.clearRect(0,0,innerWidth,innerHeight);

    const pxPerM = Math.min(innerWidth, innerHeight) * 0.18 / (l1+l2);
    const ox = innerWidth*0.5, oy = innerHeight*0.5;

    const p0 = {x: ox, y: oy};
    const p1 = {x: ox + x1*pxPerM, y: oy + y1*pxPerM};
    const p2 = {x: ox + x2*pxPerM, y: oy + y2*pxPerM};

    ctx.beginPath(); ctx.moveTo(p0.x,p0.y); ctx.lineTo(p1.x,p1.y); ctx.lineTo(p2.x,p2.y); ctx.stroke();
    ctx.beginPath(); ctx.arc(p0.x,p0.y,6,0,Math.PI*2); ctx.stroke();
    ctx.beginPath(); ctx.arc(p1.x,p1.y,10,0,Math.PI*2); ctx.stroke();
    ctx.beginPath(); ctx.arc(p2.x,p2.y,9,0,Math.PI*2); ctx.stroke();

    requestAnimationFrame(frame);
}
requestAnimationFrame(frame);
