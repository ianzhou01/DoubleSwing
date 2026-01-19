console.log("init: input/drag.js");

import { clamp, thetaFromMouse, unwrapDelta } from "../utils/math.js";
import { getCanvasPosFromClient } from "../gfx/viewport.js";

export function createDragController({ canvas, viewW, viewH, params, engine }) {
    // Drag Filter
    let dragging = 0; // 0 none, 1 bob1, 2 bob2
    let prevTh = 0;
    let hasPrev = false;
    let filtOmega = 0;

    // ====FOR TUNING====
    const ALPHA = 0.18;
    const OMEGA_MAX = 15.0;

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

    canvas.addEventListener(
        "pointerdown",
        (e) => {
            if (activePointerId !== null) return; // single pointer only
            activePointerId = e.pointerId;
            try {
                canvas.setPointerCapture(activePointerId);
            } catch {}

            lastPointerPos = getCanvasPosFromClient(canvas, e);

            // hit test bobs using current params + current positions
            const vw = viewW();
            const vh = viewH();
            const ox = vw * 0.5;
            const oy = vh * 0.5;

            const margin = 32; // px padding inside viewport
            const usable = Math.min(vw, vh) - margin * 2;

            const denom = Math.max(0.1, params.l1 + params.l2); // prevent extreme scaling
            const ppm = (usable * 0.5) / denom;

            engine.ds_update_positions(engine.h);
            const x1 = engine.ds_x1(),
                y1 = engine.ds_y1();
            const x2 = engine.ds_x2(),
                y2 = engine.ds_y2();

            const p1 = { x: ox + x1 * ppm, y: oy + y1 * ppm };
            const p2 = { x: ox + x2 * ppm, y: oy + y2 * ppm };

            const d1 = Math.hypot(lastPointerPos.x - p1.x, lastPointerPos.y - p1.y);
            const d2 = Math.hypot(lastPointerPos.x - p2.x, lastPointerPos.y - p2.y);

            // pad grab radius
            const GRAB1 = 50 + clamp(6 + 5 * Math.sqrt(params.m1), 6, 40);
            const GRAB2 = 50 + clamp(6 + 5 * Math.sqrt(params.m2), 6, 40);

            if (d2 <= GRAB2) dragging = 2;
            else if (d1 <= GRAB1) dragging = 1;
            else dragging = 0;

            hasPrev = false; // reset filter
            e.preventDefault();
        },
        { passive: false }
    );

    canvas.addEventListener(
        "pointermove",
        (e) => {
            if (e.pointerId !== activePointerId) return;
            lastPointerPos = getCanvasPosFromClient(canvas, e);
            e.preventDefault();
        },
        { passive: false }
    );

    function endPointer(e) {
        if (e.pointerId !== activePointerId) return;
        dragging = 0;
        activePointerId = null;
        lastPointerPos = null;
        hasPrev = false;
        try {
            canvas.releasePointerCapture(e.pointerId);
        } catch {}
        e.preventDefault();
    }
    canvas.addEventListener("pointerup", endPointer, { passive: false });
    canvas.addEventListener("pointercancel", endPointer, { passive: false });
    canvas.addEventListener("pointerleave", endPointer, { passive: false });

    // Keyboard reset stays in app.js
    return {
        getDragging: () => dragging,
        getLastPointerPos: () => lastPointerPos,
        resetFilter: () => {
            hasPrev = false;
        },
        updateFilteredOmega,
    };
}
