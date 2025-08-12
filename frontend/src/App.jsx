import { Routes, Route, Navigate } from "react-router-dom";
import Navbar from "./components/Navbar.jsx";
import TemperaturePage from "./pages/TemperaturePage.jsx";
import HumidityPage from "./pages/HumidityPage.jsx";
import PressurePage from "./pages/PressurePage.jsx";

export default function App() {
    return (
        <div style={{ fontFamily: "system-ui", maxWidth: 1100, margin: "0 auto" }}>
            <Navbar />
            <div style={{ padding: "1rem" }}>
                <Routes>
                    <Route path="/" element={<Navigate to="/temperature" />} />
                    <Route path="/temperature" element={<TemperaturePage />} />
                    <Route path="/humidity" element={<HumidityPage />} />
                    <Route path="/pressure" element={<PressurePage />} />
                </Routes>
            </div>
        </div>
    );
}
