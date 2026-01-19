// src/theme.js

function isLightMode() {
    return document.body.classList.contains("light");
}

export const THEME = {
    rod() {
        return isLightMode() ? "#1a2433" : "#cbd5e1";
    },
    outline() {
        return isLightMode() ? "rgb(0,0,0)" : "rgb(255,255,255)";
    },
    bob1() {
        return "#7dd3fc";
    },
    bob2() {
        return "#fda4af";
    },
    pivot() {
        return "#34d399";
    },
    bg() {
        return "rgba(255,255,255,0.0)";
    },
};
