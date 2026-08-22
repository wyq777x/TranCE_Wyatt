import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// The production build is served statically by the FastAPI sidecar
// (same origin, no CORS). In dev, Vite runs on its own port and proxies
// API calls to a manually started sidecar.
export default defineConfig({
  plugins: [react()],
  server: {
    port: 5188,
    proxy: {
      "/api": "http://127.0.0.1:9721",
      "/healthz": "http://127.0.0.1:9721",
    },
  },
  build: {
    outDir: "dist",
    sourcemap: false,
  },
});
