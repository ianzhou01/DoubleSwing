console.log("init: utils/math.js");

export function rad2deg(r) {
    return (r * 180) / Math.PI;
}

export function clamp(v, lo, hi) {
    return Math.min(hi, Math.max(lo, v));
}

export function normalizeAngle(a) {
    while (a < -Math.PI) a += 2 * Math.PI;
    while (a > Math.PI) a -= 2 * Math.PI;
    return a;
}

export function unwrapDelta(newTh, oldTh) {
    return normalizeAngle(newTh - oldTh);
}

// theta measured from vertical, +y down => atan2(x, y)
export function thetaFromMouse(dx, dy) {
    return normalizeAngle(Math.atan2(dx, dy));
}
