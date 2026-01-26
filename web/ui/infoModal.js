console.log("init: ui/infomodal.js");

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

    if (!viewport || !btn || !modal || !close) return { isOpen: () => false, open() {}, close() {} };

    let openState = false;

    function setOpen(next) {
        const prev = openState;
        openState = !!next;
        modal.classList.toggle("isOpen", openState);
        modal.setAttribute("aria-hidden", String(!openState));
        btn.setAttribute("aria-expanded", String(openState));
        if (openState) {
            // don't focus-trap; just make it usable
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
