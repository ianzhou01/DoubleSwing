console.log("init: ui/controls.js");

import { num } from "../utils/dom.js";
import { clamp } from "../utils/math.js";
import { DEFAULT_PARAMS } from "../engine/params.js";
import { setStatus } from "./elements.js";

export function setInputsFromParams(ui, p) {
    ui.l1.value = p.l1;
    ui.l2.value = p.l2;
    ui.m1.value = p.m1;
    ui.m2.value = p.m2;
    ui.damping.value = p.damping;
    ui.g.value = p.g;
}

export function readParamsFromUI(ui) {
    return {
        l1: clamp(num(ui.l1), 0.1, 50),
        l2: clamp(num(ui.l2), 0.1, 50),
        m1: clamp(num(ui.m1), 0.01, 100),
        m2: clamp(num(ui.m2), 0.01, 100),
        g: clamp(num(ui.g), 0.0, 50),
        damping: clamp(num(ui.damping), 0.0, 5.0),
    };
}

// binds click handlers; inject dependencies to avoid circular imports
export function bindUIControls({
                                   ui,
                                   statusEl,
                                   params,
                                   engine,
                                   syncEngineParams,
                                   resetFiltersAndTiming,
                                   getDragging,
                               }) {
    ui.apply.addEventListener("click", () => {
        if (getDragging() !== 0) return;

        Object.assign(params, readParamsFromUI(ui));
        setInputsFromParams(ui, params);
        syncEngineParams(engine, params);
        resetFiltersAndTiming();
        setStatus(statusEl, "Applied parameters.", "ok");
    });

    ui.defaults.addEventListener("click", () => {
        if (getDragging() !== 0) return;

        engine.ds_reset(engine.h, -0.1, 0.0);

        Object.assign(params, DEFAULT_PARAMS);
        setInputsFromParams(ui, params);

        syncEngineParams(engine, params);
        resetFiltersAndTiming();

        setStatus(statusEl, "Restored default parameters.", "ok");
    });

    ui.reset.addEventListener("click", () => {
        engine.ds_reset(engine.h, -0.1, 0.0);
        resetFiltersAndTiming();
        setStatus(statusEl, "Reset animation.", "ok");
    });
}
