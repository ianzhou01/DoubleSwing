console.log("init: ui/themeToggle.js");

const STORAGE_KEY = "doubleswing:theme"; // "light" | "dark"

function applyTheme(mode) {
    document.body.classList.toggle("light", mode === "light");
}

function modeFromCheckbox(checked) {
    return checked ? "light" : "dark";
}

function modeToCheckbox(mode) {
    return mode === "light";
}

export function initThemeToggle({ checkboxId = "lightMode" } = {}) {
    const cb = document.getElementById(checkboxId);
    if (!cb) return { getMode: () => "dark", setMode: () => {} };

    // initial mode: saved preference > dark
    const saved = localStorage.getItem(STORAGE_KEY);

    const initialMode = saved ?? "dark";

    cb.checked = modeToCheckbox(initialMode);
    applyTheme(initialMode);

    cb.addEventListener("change", () => {
        const mode = modeFromCheckbox(cb.checked);
        applyTheme(mode);
        localStorage.setItem(STORAGE_KEY, mode);
    });

    return {
        getMode: () => (document.body.classList.contains("light") ? "light" : "dark"),
        setMode: (mode) => {
            cb.checked = modeToCheckbox(mode);
            applyTheme(mode);
            localStorage.setItem(STORAGE_KEY, mode);
        },
    };
}
