import { NavLink } from "react-router-dom";
import skyBackground from "../assets/images/sky-background.jpg"; // tilpas stien

const link = {
    padding: "0.5rem 1rem",
    textDecoration: "none",
    color: "#222",
    borderRadius: 8,
    outline: "none",
    boxShadow: "none",
};


export default function Navbar() {
    return (
        <nav
            style={{
                position: "sticky",
                top: 0,
                zIndex: 10,
                backgroundImage: `url(${skyBackground})`,
                backgroundSize: "cover",
                backgroundPosition: "left center",
                backgroundRepeat: "no-repeat",
                padding: "1.5rem 1rem",
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
                gap: 8
            }}
        >
            <h1 style={{ margin: 0, color: "#fff", textShadow: "1px 1px 3px rgba(0,0,0,0.6)" }}>
                Vejrstation
            </h1>
            <div style={{ display: "flex", gap: 12 }}>
                {[
                    { path: "/temperature", label: "Temperatur" },
                    { path: "/humidity", label: "Fugtighed" },
                    { path: "/pressure", label: "Tryk" }
                ].map(({ path, label }) => (
                    <NavLink
                        key={path}
                        to={path}
                        style={({ isActive }) => ({
                            ...link,
                            color: isActive ? "black" : "#fff",
                            textDecoration: isActive ? "underline" : "none"
                        })}
                        onMouseEnter={(e) => e.currentTarget.style.textDecoration = "underline"}
                        onMouseLeave={(e) => {
                            if (!e.currentTarget.classList.contains("active")) {
                                e.currentTarget.style.textDecoration = "none";
                            }
                        }}
                    >
                        {label}
                    </NavLink>
                ))}
            </div>
        </nav>
    );
}
