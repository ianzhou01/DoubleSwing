console.log("init: gfx/draw.js");

import { THEME } from "../theme.js";
import { clamp } from "../utils/math.js";
import { imageReady, tryRetrySprite } from "./sprites.js";

export function filledCircle(ctx, p, r, fill, outline = THEME.outline()) {
    ctx.beginPath();
    ctx.arc(p.x, p.y, r, 0, Math.PI * 2);
    ctx.fillStyle = fill;
    ctx.fill();
    ctx.lineWidth = 2;
    ctx.strokeStyle = outline;
    ctx.stroke();
}

export function drawSpriteOrFallback(ctx, sprite, p, r, angleRad, fallbackFill) {
    if (imageReady(sprite)) {
        const size = r * 2;

        ctx.save();
        ctx.translate(p.x, p.y);
        ctx.rotate(-angleRad);
        ctx.drawImage(sprite.img, -r, -r, size, size);
        ctx.restore();
        return;
    }

    if (sprite && sprite.status === "err") {
        tryRetrySprite(sprite);
    }

    filledCircle(ctx, p, r, fallbackFill);
}

export function makeDrawer({ ctx, viewW, viewH, params, engine, sprites }) {
    return function draw(x1, y1, x2, y2) {
        ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);

        const ox = viewW() * 0.5;
        const oy = viewH() * 0.5;

        const margin = 32;
        const usable = Math.min(viewW(), viewH()) - margin * 2;
        const ppm = (usable * 0.5) / (params.l1 + params.l2);

        const p0 = { x: ox, y: oy };
        const p1 = { x: ox + x1 * ppm, y: oy + y1 * ppm };
        const p2 = { x: ox + x2 * ppm, y: oy + y2 * ppm };

        const snap = (v) => Math.round(v) + 0.5;
        p0.x = snap(p0.x);
        p0.y = snap(p0.y);
        p1.x = snap(p1.x);
        p1.y = snap(p1.y);
        p2.x = snap(p2.x);
        p2.y = snap(p2.y);

        ctx.lineWidth = 2;
        ctx.strokeStyle = THEME.rod();
        ctx.beginPath();
        ctx.moveTo(p0.x, p0.y);
        ctx.lineTo(p1.x, p1.y);
        ctx.lineTo(p2.x, p2.y);
        ctx.stroke();

        const r1 = clamp(6 + 5 * Math.sqrt(params.m1), 10, 40);
        const r2 = clamp(6 + 5 * Math.sqrt(params.m2), 10, 40);

        const th1v = engine.ds_th1(engine.h);
        const th2v = engine.ds_th2(engine.h);

        filledCircle(ctx, p0, 6, THEME.pivot());
        drawSpriteOrFallback(ctx, sprites.bob1, p1, r1, th1v, THEME.bob1());
        drawSpriteOrFallback(ctx, sprites.bob2, p2, r2, th2v, THEME.bob2());
    };
}
