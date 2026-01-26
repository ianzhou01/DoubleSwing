console.log("init: ui/infoModal.js");

import { loadInfoHTML } from "./infoLoader.js";

export function initInfoModal({
    viewportId = "viewport",
    btnId = "infoBtn",
    modalId = "infoModal",
    closeId = "infoClose",
    onToggle = null,
} = {}) {
    const viewport = document.getElementById(viewportId);
    const btn = document.getElementById(btnId);
    const modal = document.getElementById(modalId);
    const close = document.getElementById(closeId);

    if (!viewport || !btn || !modal || !close) {
        return { isOpen: () => false, open() {}, close() {} };
    }

    const body = modal.querySelector(".infoModal__body");

    let openState = false;
    let injected = false;
    let loading = false;

    async function ensureContent() {
        if (!body || injected || loading) return;
        loading = true;

        try {
            body.innerHTML = `<p style="opacity:.8">Loading…</p>`;

            // Default: load web/ui/info.md via import.meta.url in infoLoader.js
            const html = await loadInfoHTML();

            body.innerHTML = html;
            injected = true;

            // Run KaTeX auto-render on subtree
            if (window.renderMathInElement) {
                window.renderMathInElement(body, {
                    delimiters: [
                        { left: "$$", right: "$$", display: true },
                        { left: "$", right: "$", display: false },
                    ],
                });
            }
        } catch (e) {
            body.innerHTML = `
                <p><strong>Failed to load info.</strong></p>
                <p style="opacity:.8">${String(e?.message ?? e)}</p>
            `;
        } finally {
            loading = false;
        }
    }

    function setOpen(next) {
        const prev = openState;
        openState = !!next;

        modal.classList.toggle("isOpen", openState);
        modal.setAttribute("aria-hidden", String(!openState));
        btn.setAttribute("aria-expanded", String(openState));

        if (openState) {
            ensureContent();
            close.focus?.();
        } else {
            btn.focus?.();
        }
        if (prev !== openState) {
            try { onToggle?.(openState); } catch {}
        }
    }

    function stopAll(e) {
        e.preventDefault();
        e.stopPropagation();
    }

    btn.addEventListener("pointerdown", (e) => { stopAll(e); setOpen(true); }, { passive: false });
    btn.addEventListener("click", (e) => { e.preventDefault(); e.stopPropagation(); setOpen(true); });

    close.addEventListener("pointerdown", (e) => { stopAll(e); setOpen(false); }, { passive: false });
    close.addEventListener("click", (e) => { e.preventDefault(); e.stopPropagation(); setOpen(false); });

    // click on tinted backdrop closes
    modal.addEventListener("pointerdown", (e) => {
        // only close if they clicked the backdrop, not inside the panel
        if (e.target === modal) { stopAll(e); setOpen(false); }
    }, { passive: false });

    // ESC closes (no freeze)
    window.addEventListener("keydown", (e) => {
        if (e.key === "Escape" && openState) setOpen(false);
    });

    // block pointer events from reaching canvas when open
    modal.addEventListener("pointermove", stopAll, { passive: false });
    modal.addEventListener("wheel", (e) => { e.stopPropagation(); }, { passive: true });

    return {
        isOpen: () => openState,
        open: () => setOpen(true),
        close: () => setOpen(false),
    };
}
