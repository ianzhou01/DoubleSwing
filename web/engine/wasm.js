console.log("init: engine/wasm.js");

import createModule from "../doubleswing.js";

export async function initEngine(params) {
    const mod = await createModule();

    // core
    const ds_create = mod.cwrap("ds_create", "number", [
        "number",
        "number",
        "number",
        "number",
        "number",
        "number",
        "number",
        "number",
        "number",
        "number",
    ]);
    const ds_destroy = mod.cwrap("ds_destroy", null, ["number"]);
    const ds_step = mod.cwrap("ds_step", null, ["number", "number"]);
    const ds_step_drag_p1 = mod.cwrap("ds_step_drag_p1", null, [
        "number",
        "number",
        "number",
        "number",
        "number",
    ]); // h,dt,th1,w1,a1
    const ds_update_positions = mod.cwrap("ds_update_positions", null, ["number"]);

    // cached positions
    const ds_x1 = mod.cwrap("ds_x1", "number", []);
    const ds_y1 = mod.cwrap("ds_y1", "number", []);
    const ds_x2 = mod.cwrap("ds_x2", "number", []);
    const ds_y2 = mod.cwrap("ds_y2", "number", []);

    // state setters
    const ds_set_th2 = mod.cwrap("ds_set_th2", null, ["number", "number"]);
    const ds_set_w2 = mod.cwrap("ds_set_w2", null, ["number", "number"]);
    const ds_reset = mod.cwrap("ds_reset", null, ["number", "number", "number", "number", "number"]);

    // param setters
    const ds_set_l1 = mod.cwrap("ds_set_l1", null, ["number", "number"]);
    const ds_set_l2 = mod.cwrap("ds_set_l2", null, ["number", "number"]);
    const ds_set_m1 = mod.cwrap("ds_set_m1", null, ["number", "number"]);
    const ds_set_m2 = mod.cwrap("ds_set_m2", null, ["number", "number"]);
    const ds_set_damping = mod.cwrap("ds_set_damping", null, ["number", "number"]);
    const ds_set_g = mod.cwrap("ds_set_g", null, ["number", "number"]);

    // getters (for readout)
    const ds_th1 = mod.cwrap("ds_th1", "number", ["number"]);
    const ds_w1 = mod.cwrap("ds_w1", "number", ["number"]);
    const ds_th2 = mod.cwrap("ds_th2", "number", ["number"]);
    const ds_w2 = mod.cwrap("ds_w2", "number", ["number"]);

    const ds_ke = mod.cwrap("ds_ke", "number", ["number"]);
    const ds_pe = mod.cwrap("ds_pe", "number", ["number"]);
    const ds_energy = mod.cwrap("ds_energy", "number", ["number"]);

    const h = ds_create(
        params.l1,
        params.l2,
        params.m1,
        params.m2,
        params.g,
        params.damping,
        params.th1,
        params.w1,
        params.th2,
        params.w2
    );

    return {
        mod,
        h,
        // core
        ds_create,
        ds_destroy,
        ds_step,
        ds_step_drag_p1,
        ds_update_positions,
        // positions
        ds_x1,
        ds_y1,
        ds_x2,
        ds_y2,
        // setters
        ds_set_th2,
        ds_set_w2,
        ds_reset,
        ds_set_l1,
        ds_set_l2,
        ds_set_m1,
        ds_set_m2,
        ds_set_damping,
        ds_set_g,
        // getters
        ds_th1,
        ds_w1,
        ds_th2,
        ds_w2,
        ds_ke,
        ds_pe,
        ds_energy,
    };
}
