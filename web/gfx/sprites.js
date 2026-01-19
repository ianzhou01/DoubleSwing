console.log("init: gfx/sprites.js");

export function loadSprite(src) {
    const img = new Image();
    // img.crossOrigin = "anonymous"; // only if truly cross-origin and server sends CORS headers

    const sprite = {
        img,
        src,
        status: "loading", // "loading" | "ok" | "err"
        attempts: 0,
        maxAttempts: 3,
        nextRetryAt: 0, // performance.now() timestamp
    };

    img.addEventListener("load", () => {
        sprite.status = img.naturalWidth > 0 ? "ok" : "err";
    });
    img.addEventListener("error", () => {
        sprite.status = "err";
    });

    // small perf hint
    img.decoding = "async";
    img.src = src;

    return sprite;
}

export function tryRetrySprite(sprite, now = performance.now()) {
    if (!sprite) return;
    if (sprite.status !== "err") return;
    if (sprite.attempts >= sprite.maxAttempts) return;
    if (now < sprite.nextRetryAt) return;

    sprite.attempts++;
    sprite.status = "loading";

    // exponential-ish backoff
    const delay = 250 * Math.pow(2, sprite.attempts - 1);
    sprite.nextRetryAt = now + delay;

    // cache-bust only on retry
    sprite.img.src = `${sprite.src}?v=${Date.now()}`;
}

export function imageReady(sprite) {
    return sprite && sprite.status === "ok" && sprite.img.complete && sprite.img.naturalWidth > 0;
}
