// Dansk tid uden AM/PM
export function formatTime(t) {
    return new Date(t).toLocaleTimeString("da-DK", {
        hour: "2-digit",
        minute: "2-digit",
    });
}

// Dansk dato + tid (til tooltip)
export function formatDateTime(t) {
    return new Date(t).toLocaleString("da-DK", {
        day: "2-digit",
        month: "2-digit",
        year: "numeric",
        hour: "2-digit",
        minute: "2-digit",
    });
}

export function formatDateTimeAxis(t) {
    const d = new Date(t);
    const pad = (n) => String(n).padStart(2, "0");
    return `${pad(d.getDate())}/${pad(d.getMonth() + 1)} ${pad(d.getHours())}:${pad(d.getMinutes())}`;
}


