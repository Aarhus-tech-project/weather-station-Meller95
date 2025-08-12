// HumidityPage.jsx
import { useQuery } from "@tanstack/react-query";
import { useMemo, useState } from "react";
import { ResponsiveContainer, LineChart, Line, Tooltip, XAxis, YAxis } from "recharts";
import DatePicker from "react-datepicker";

import { getLatest, getInterval, isoHoursAgo, isoNow } from "../lib/api.js";
import { formatDateTime, formatDateTimeAxis } from "../lib/formatters.js";
import StatCard from "../components/StatCard.jsx";

function makeTicks(startMs, endMs, n = 10) {
    if (!isFinite(startMs) || !isFinite(endMs) || endMs <= startMs || n < 2) return [];
    const step = (endMs - startMs) / (n - 1);
    return Array.from({ length: n }, (_, i) => Math.round(startMs + i * step));
}

export default function HumidityPage() {
    // init: seneste 2 timer
    const initFromIso = isoHoursAgo(2);
    const initToIso = isoNow();

    // Date pickers
    const [fromDt, setFromDt] = useState(new Date(initFromIso));
    const [toDt, setToDt] = useState(new Date(initToIso));

    // ISO til API
    const [fromIso, setFromIso] = useState(initFromIso);
    const [toIso, setToIso] = useState(initToIso);

    const latestQ = useQuery({ queryKey: ["latest"], queryFn: getLatest, refetchInterval: 30000 });
    const intervalQ = useQuery({
        queryKey: ["hum-range", fromIso, toIso],
        queryFn: () => getInterval(fromIso, toIso),
        refetchInterval: Math.abs(new Date(toIso).getTime() - Date.now()) < 60_000 ? 30000 : false,
    });

    const latest = latestQ.data ?? { temperatur: 0, luftfugtighed: 0, tryk: 0 };
    const data = intervalQ.data ?? [];

    // Numerisk tidsakse
    const series = useMemo(() => data.map(d => ({ ...d, t: new Date(d.tidspunkt).getTime() })), [data]);
    const start = series[0]?.t ?? Date.now();
    const end = series.at(-1)?.t ?? start;
    const span = Math.max(1, end - start);
    const pad = span * 0.01;
    const ticks = useMemo(() => makeTicks(start, end, 10), [start, end]);

    function applyRange() {
        setFromIso(fromDt.toISOString());
        setToIso(toDt.toISOString());
    }

    const isLoading = latestQ.isLoading || intervalQ.isLoading;
    const error = latestQ.error || intervalQ.error;

    return (
        <>
            <h2>Fugtighed</h2>

            <div style={{ display: "grid", gridTemplateColumns: "repeat(3, 1fr)", gap: 16 }}>
                <StatCard title="Seneste fugtighed" value={latest.luftfugtighed?.toFixed(2) ?? "0.00"} unit="%" />
                <StatCard title="Seneste temperatur" value={latest.temperatur?.toFixed(2) ?? "0.00"} unit={'\u00B0C'} />
                <StatCard title="Seneste tryk" value={latest.tryk?.toFixed(2) ?? "0.00"} unit="hPa" />
            </div>

            <div style={{ display: "flex", gap: 12, alignItems: "end", marginBottom: 12, marginTop: 20 }}>
                <label style={{ display: "grid", gap: 4 }}>
                    Fra:
                    <DatePicker
                        selected={fromDt}
                        onChange={(d) => d && setFromDt(d)}
                        showTimeSelect
                        timeIntervals={5}
                        timeCaption="Tid"
                        timeFormat="HH:mm"
                        dateFormat="dd/MM/yyyy HH:mm"
                        locale="da"
                        className="rp-button"
                    />
                </label>
                <label style={{ display: "grid", gap: 4 }}>
                    Til:
                    <DatePicker
                        selected={toDt}
                        onChange={(d) => d && setToDt(d)}
                        showTimeSelect
                        timeIntervals={5}
                        timeCaption="Tid"
                        timeFormat="HH:mm"
                        dateFormat="dd/MM/yyyy HH:mm"
                        locale="da"
                        className="rp-button"
                    />
                </label>
                <button onClick={applyRange} className="rp-button">Vis data</button>
                <button
                    onClick={() => {
                        const fIso = isoHoursAgo(2);
                        const tIso = isoNow();
                        setFromIso(fIso); setToIso(tIso);
                        setFromDt(new Date(fIso)); setToDt(new Date(tIso));
                    }}
                    className="rp-button"
                >
                    Seneste 2 timer
                </button>
            </div>

            {error && <p>Fejl: {String(error)}</p>}
            {isLoading && <p>Henter data…</p>}

            {!isLoading && !error && (
                <div style={{ marginTop: 30 }}>
                    <ResponsiveContainer width="100%" height={320}>
                        <LineChart data={series} margin={{ top: 8, right: 36, bottom: 8, left: 8 }}>
                            <XAxis
                                dataKey="t"
                                type="number"
                                scale="time"
                                domain={[start - pad, end + pad]}
                                ticks={ticks}
                                interval={0}
                                tickFormatter={formatDateTimeAxis}
                                tickMargin={8}
                            />
                            <YAxis unit="%" />
                            <Tooltip labelFormatter={(ms) => formatDateTime(ms)} />
                            <Line type="monotone" dataKey="luftfugtighed" dot={false} />
                        </LineChart>
                    </ResponsiveContainer>
                </div>
            )}
        </>
    );
}
