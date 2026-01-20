console.log("init: ui/readout.js");

import { rad2deg } from "../utils/math.js";

export function makeReadoutUpdater({ ui, params, engine, getDragging }) {
    return function updateReadout() {
        // positions already updated in frame()
        const th1v = engine.ds_th1(engine.h), w1v = engine.ds_w1(engine.h);
        const th2v = engine.ds_th2(engine.h), w2v = engine.ds_w2(engine.h);
        const x1 = engine.ds_x1(), y1 = engine.ds_y1(), x2 = engine.ds_x2(), y2 = engine.ds_y2();
        const ke = engine.ds_ke(engine.h), pe = engine.ds_pe(engine.h);
        const E = engine.ds_energy(engine.h);
        const dragging = getDragging();

        ui.readout.innerHTML = `
L1=${params.l1.toFixed(2)}  L2=${params.l2.toFixed(2)}  M1=${params.m1.toFixed(2)}  M2=${params.m2.toFixed(2)}
g=${params.g.toFixed(3)}  damping=${params.damping.toFixed(3)}

<span class="math">θ</span>1: ${rad2deg(th1v).toFixed(2)} deg   <span class="math">ω</span>1: ${w1v.toFixed(3)} rad/s
<span class="math">θ</span>2: ${rad2deg(th2v).toFixed(2)} deg   <span class="math">ω</span>2: ${w2v.toFixed(3)} rad/s

x1: ${x1.toFixed(3)}  y1: ${(-y1).toFixed(3)}
x2: ${x2.toFixed(3)}  y2: ${(-y2).toFixed(3)}

Total Energy (N): ${E.toFixed(4)}
Kinetic Energy (N): ${ke.toFixed(4)}
Potential Energy (N): ${pe.toFixed(4)}
Currently dragging: ${dragging === 1 ? "Bob 1" : dragging === 2 ? "Bob 2" : "none"}`.trim();
    };
}
