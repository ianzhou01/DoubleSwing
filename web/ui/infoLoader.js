console.log("init: ui/infoLoader.js");
let cache = {
    loaded: false,
    html: "",
    err: null,
};

export async function loadInfoHTML({ mdPath = null } = {}) {
    if (cache.loaded) return cache.html;
    if (cache.err) throw cache.err;

    try {
        const url = mdPath
            ? new URL(mdPath, document.baseURI)     // caller-provided path
            : new URL("./info.md", import.meta.url); // default alongside infoLoader.js

        const res = await fetch(url, { cache: "no-store" });
        if (!res.ok) throw new Error(`Failed to load ${url}: ${res.status} ${res.statusText}`);

        const md = await res.text();

        // Parse markdown -> HTML
        const marked = window.marked;
        if (!marked) throw new Error("Markdown parser not found. Check vendors/marked.min.js load status.");

        marked.setOptions({ gfm: true, breaks: false });

        const html = marked.parse(md);

        cache.loaded = true;
        cache.html = html;
        return html;
    } catch (e) {
        cache.err = e;
        throw e;
    }
}

export function clearInfoCache() {
    cache.loaded = false;
    cache.html = "";
    cache.err = null;
}
