console.log("init: engine/params.js");

export const DEFAULT_PARAMS = {
    l1: 2.0,
    l2: 2.0,
    m1: 1.0,
    m2: 1.0,
    g: 9.80665,
    damping: 0.02,
};

export function makeParams() {
    return { ...DEFAULT_PARAMS };
}

export function syncEngineParams(engine, params) {
    engine.ds_set_l1(engine.h, params.l1);
    engine.ds_set_l2(engine.h, params.l2);
    engine.ds_set_m1(engine.h, params.m1);
    engine.ds_set_m2(engine.h, params.m2);
    engine.ds_set_damping(engine.h, params.damping);
    engine.ds_set_g(engine.h, params.g);
}
