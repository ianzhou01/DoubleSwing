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

export function setStatus(statusEl, msg, kind = "") {
    statusEl.textContent = msg;
    statusEl.className = "status" + (kind ? ` ${kind}` : "");
}
