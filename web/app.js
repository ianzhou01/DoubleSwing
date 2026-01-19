console.log("init: app.js");

import { loadSprite } from "./gfx/sprites.js";
import { setupCanvasResize } from "./gfx/viewport.js";
import { initEngine } from "./engine/wasm.js";
import { makeParams, syncEngineParams } from "./engine/params.js";
import { getUIElements, getStatusEl, getEnergyBarEls } from "./ui/elements.js";
import { initThemeToggle } from "./ui/themeToggle.js";
import { setInputsFromParams, bindUIControls } from "./ui/controls.js";
import { makeReadoutUpdater } from "./ui/readout.js";
import { makeEnergyBarUpdater } from "./ui/energyBar.js";
import { createDragController } from "./input/drag.js";
import { makeDrawer } from "./gfx/draw.js";
import { thetaFromMouse } from "./utils/math.js";

// DOM
const canvas = document.getElementById("c");
const viewport = document.getElementById("viewport");
const ctx = canvas.getContext("2d");

// resize + helpers
const { viewW, viewH } = setupCanvasResize({ canvas, viewport, ctx });

// status + energy bar els
const statusEl = getStatusEl();
const energyEls = getEnergyBarEls();

// sprites
const bob1Sprite = loadSprite("./assets/bob1.png");
const bob2Sprite = loadSprite("./assets/bob2.png");
const sprites = { bob1: bob1Sprite, bob2: bob2Sprite };

// params + engine
const params = makeParams();
const engine = await initEngine(params);

// UI
const ui = getUIElements();
initThemeToggle({ checkboxId: "lightMode" });

// initialize inputs from params
setInputsFromParams(ui, params);

// ensure engine params match
syncEngineParams(engine, params);

// timing/filters
let last = performance.now();
let acc = 0.0;

const FIXED_DT = 1 / 240;
const MAX_FRAME = 1 / 15;
const MAX_STEPS = 8;

function resetFiltersAndTiming(dragController) {
    dragController.resetFilter();
    acc = 0.0; // prevents burst of steps after UI changes
}

// drag controller
const drag = createDragController({ canvas, viewW, viewH, params, engine });

// bind UI buttons
bindUIControls({
    ui,
    statusEl,
    params,
    engine,
    syncEngineParams,
    resetFiltersAndTiming: () => resetFiltersAndTiming(drag),
    getDragging: drag.getDragging,
});

// keyboard reset
addEventListener("keydown", (e) => {
    if (e.key === "r" || e.key === "R") {
        engine.ds_reset(engine.h, -0.1, 0.0);
        resetFiltersAndTiming(drag);
    }
});

// readout + energy bar
const updateReadout = makeReadoutUpdater({ ui, params, engine, getDragging: drag.getDragging });
const updateEnergyBar = makeEnergyBarUpdater({ engine, els: energyEls });

// draw
const draw = makeDrawer({ ctx, viewW, viewH, params, engine, sprites });

// animation
function frame(t) {
    let frameDt = (t - last) / 1000;
    last = t;

    frameDt = Math.min(frameDt, MAX_FRAME);
    acc += frameDt;

    const mp = drag.getLastPointerPos();
    const dragging = drag.getDragging();

    const ox = viewW() * 0.5;
    const oy = viewH() * 0.5;

    const margin = 32;
    const usable = Math.min(viewW(), viewH()) - margin * 2;
    const ppm = (usable * 0.5) / (params.l1 + params.l2);

    // needed for bob2 dragging
    let p1px = null;
    if (dragging === 2 && mp) {
        engine.ds_update_positions(engine.h);
        const x1 = engine.ds_x1(),
            y1 = engine.ds_y1();
        p1px = { x: ox + x1 * ppm, y: oy + y1 * ppm };
    }

    let steps = 0;
    while (acc >= FIXED_DT && steps < MAX_STEPS) {
        if (dragging === 1 && mp) {
            const dx = mp.x - ox;
            const dy = mp.y - oy;
            const th1 = thetaFromMouse(dx, dy);
            let w1 = drag.updateFilteredOmega(th1, FIXED_DT);
            if (Math.abs(w1) < 0.05) w1 = 0.0;
            const a1 = 0.0;
            engine.ds_step_drag_p1(engine.h, FIXED_DT, th1, w1, a1);
        } else {
            if (dragging === 2 && mp && p1px) {
                const dx = mp.x - p1px.x;
                const dy = mp.y - p1px.y;
                const th2 = thetaFromMouse(dx, dy);
                const w2 = drag.updateFilteredOmega(th2, FIXED_DT);
                engine.ds_set_th2(engine.h, th2);
                engine.ds_set_w2(engine.h, w2);
            }
            engine.ds_step(engine.h, FIXED_DT);
        }

        steps++;
        acc -= FIXED_DT;
    }

    engine.ds_update_positions(engine.h);
    const x1 = engine.ds_x1(),
        y1 = engine.ds_y1(),
        x2 = engine.ds_x2(),
        y2 = engine.ds_y2();

    draw(x1, y1, x2, y2);
    updateReadout();
    updateEnergyBar();

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);

// cleanup
addEventListener("beforeunload", () => {
    try {
        engine.ds_destroy(engine.h);
    } catch {}
});
