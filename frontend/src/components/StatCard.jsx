export default function StatCard({ title, value, unit }) {
    return (
        <div style={{ border: "1px solid #e0e0e0", borderRadius: 12, padding: 16 }}>
            <div style={{ fontSize: 14, color: "#666" }}>{title}</div>
            <div style={{ fontSize: 32, fontWeight: 600 }}>{value}{unit ? ` ${unit}` : ""}</div>
        </div>
    );
}
