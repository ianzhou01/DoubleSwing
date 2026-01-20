console.log("init: ui/elements.js");

export function getUIElements() {
    return {
        l1: document.getElementById("l1"),
        l2: document.getElementById("l2"),
        m1: document.getElementById("m1"),
        m2: document.getElementById("m2"),
        damping: document.getElementById("damping"),
        g: document.getElementById("g"),
        apply: document.getElementById("apply"),
        reset: document.getElementById("reset"),
        defaults: document.getElementById("defaults"),
        readout: document.getElementById("readout"),
    };
}

export function getStatusEl() {
    return document.getElementById("status");
}

export function getEnergyBarEls() {
    return {
        keBar: document.getElementById("keBar"),
        peBar: document.getElementById("peBar"),
        eDivider: document.getElementById("eDivider"),
    };
}

let statusTimer = null;
let clearTimer = null;

export function setStatus(statusEl, msg, kind = "") {
    statusEl.classList.remove("status--gone");

    // ensure wrapper exists
    if (!statusEl.firstElementChild || !statusEl.firstElementChild.classList.contains("status__inner")) {
        statusEl.innerHTML = `<div class="status__inner"></div>`;
    }
    const inner = statusEl.querySelector(".status__inner");

    if (statusTimer) clearTimeout(statusTimer);
    if (clearTimer) clearTimeout(clearTimer);

    // reset classes
    statusEl.classList.remove("status--shown", "ok", "err");
    inner.textContent = msg;
    if (kind) statusEl.classList.add(kind);

    // force transition restart
    void statusEl.offsetHeight;

    requestAnimationFrame(() => statusEl.classList.add("status--shown"));

    // hide after 5 seconds
    statusTimer = setTimeout(() => {
        statusEl.classList.remove("status--shown");

        clearTimer = setTimeout(() => {
            inner.textContent = "";
            statusEl.classList.remove("ok", "err");

            statusEl.classList.add("status--gone");

            statusTimer = null;
            clearTimer = null;
        }, 300);
    }, 5000);
}
