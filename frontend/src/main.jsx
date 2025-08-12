import React from "react";
import ReactDOM from "react-dom/client";
import { BrowserRouter } from "react-router-dom";
import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import App from "./App.jsx";
import 'react-datepicker/dist/react-datepicker.css';
import "./index.css";
import { registerLocale } from "react-datepicker";
import da from "date-fns/locale/da";
registerLocale("da", da);


const qc = new QueryClient();

ReactDOM.createRoot(document.getElementById("root")).render(
    <BrowserRouter>
        <QueryClientProvider client={qc}>
            <App />
        </QueryClientProvider>
    </BrowserRouter>
);
