// YYYY-MM-DDTHH:MM til inputfelt (fra ISO eller Date)
export function toInputValue(isoOrDate) {
    const d = typeof isoOrDate === "string" ? new Date(isoOrDate) : isoOrDate;
    const pad = (n) => String(n).padStart(2, "0");
    return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}T${pad(d.getHours())}:${pad(d.getMinutes())}`;
}

// Fra inputværdi (local) til ISO (UTC) til API'et
export function fromInputValueToIso(inputValue) {
    // inputValue er uden timezone fortolkes som lokal tid af Date(...)
    return new Date(inputValue).toISOString();
}
