import React from "react";
import ReactDOM from "react-dom/client";
import { ViewerApp } from "./components/Viewer/Viewer.tsx";

ReactDOM.createRoot(document.getElementById("root")!).render(
    <React.StrictMode>
        <ViewerApp />
    </React.StrictMode>,
);
