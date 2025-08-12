const API_BASE = import.meta.env.VITE_API_BASE; // read from .env.development

async function http(url) {
    const r = await fetch(url);
    if (!r.ok) {
        const txt = await r.text().catch(() => "");
        throw new Error(`${r.status} ${r.statusText}: ${txt}`);
    }
    return r.json();
}

export function getLatest() {
    return http(`${API_BASE}/seneste`);
}

export function getInterval(fromIso, toIso) {
    const qs = new URLSearchParams({ from: fromIso, to: toIso }).toString();
    return http(`${API_BASE}/interval?${qs}`);
}

export function isoNow() {
    return new Date().toISOString();
}

export function isoHoursAgo(h) {
    const now = new Date();
    return new Date(now.getTime() - h * 60 * 60 * 1000).toISOString();
}
