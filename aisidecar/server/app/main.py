"""TranCE AI sidecar application factory.

Security model: the sidecar binds 127.0.0.1 only, and every /api request
must carry the one-time bearer token injected by the Qt host process.
Static web assets and /healthz are open (they carry no user data); the
token reaches the web UI through the page URL and is kept for API calls.
"""

from __future__ import annotations

from pathlib import Path

from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse, FileResponse
from fastapi.staticfiles import StaticFiles

from .api import (
    chat_router,
    memory_router,
    rag_router,
    session_router,
    sync_router,
)
from .config import VERSION, Config

STATIC_OPEN_PREFIXES = ("/healthz", "/assets", "/favicon")


def create_app(config: Config) -> FastAPI:
    app = FastAPI(title="TranCE AI Sidecar", version=VERSION)
    app.state.config = config

    @app.exception_handler(404)
    async def not_found(request: Request, exc):  # noqa: ANN001
        # SPA history fallback: unknown GET paths without an extension
        # serve index.html so client-side routes work.
        path = request.url.path
        if (
            request.method == "GET"
            and not path.startswith("/api")
            and "." not in path.rsplit("/", 1)[-1]
            and (config.web_dist_dir / "index.html").exists()
        ):
            return FileResponse(config.web_dist_dir / "index.html")
        return JSONResponse({"detail": "not found"}, status_code=404)

    @app.middleware("http")
    async def token_guard(request: Request, call_next):
        path = request.url.path

        if path == "/healthz" or path.startswith(STATIC_OPEN_PREFIXES):
            return await call_next(request)

        if path.startswith("/api"):
            auth = request.headers.get("authorization", "")

            if auth != f"Bearer {config.token}":
                return JSONResponse(
                    {"detail": "unauthorized"}, status_code=401
                )

        return await call_next(request)

    @app.get("/healthz")
    def healthz() -> dict:
        return {"status": "ok", "version": VERSION}

    app.include_router(session_router)
    app.include_router(chat_router)
    app.include_router(sync_router)
    app.include_router(memory_router)
    app.include_router(rag_router)

    if (config.web_dist_dir / "index.html").exists():
        app.mount(
            "/",
            StaticFiles(directory=config.web_dist_dir, html=True),
            name="web",
        )

    return app
