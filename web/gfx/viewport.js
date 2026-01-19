console.log("init: gfx/viewport.js");

export function setupCanvasResize({ canvas, viewport, ctx }) {
    canvas.style.touchAction = "none";

    function resize() {
        const dpr = window.devicePixelRatio || 1;

        const rect = viewport.getBoundingClientRect();
        const w = Math.max(1, Math.floor(rect.width));
        const h = Math.max(1, Math.floor(rect.height));

        canvas.width = Math.floor(w * dpr);
        canvas.height = Math.floor(h * dpr);

        // draw in CSS pixels
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    }

    function viewW() {
        return viewport.getBoundingClientRect().width;
    }
    function viewH() {
        return viewport.getBoundingClientRect().height;
    }

    addEventListener("resize", resize);
    resize();

    return { resize, viewW, viewH };
}

export function getCanvasPosFromClient(canvas, e) {
    const rect = canvas.getBoundingClientRect();
    return { x: e.clientX - rect.left, y: e.clientY - rect.top };
}
