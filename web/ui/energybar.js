console.log("init: ui/energybar.js");

export function makeEnergyBarUpdater({ engine, els }) {
    return function updateEnergyBar() {
        const KE = engine.ds_ke(engine.h);
        const PE = engine.ds_pe(engine.h);

        // Shift PE so "visual PE energy" is >= 0 (stable proportions)
        const E = KE + PE;

        let keFrac = 0.5,
            peFrac = 0.5;
        if (E > 1e-3) {
            keFrac = KE / E;
            peFrac = PE / E;
        }

        els.keBar.style.width = `${(keFrac * 100).toFixed(2)}%`;
        els.peBar.style.width = `${(peFrac * 100).toFixed(2)}%`;
        els.eDivider.style.left = `${(keFrac * 100).toFixed(2)}%`;
    };
}
