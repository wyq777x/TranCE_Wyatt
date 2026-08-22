"""Runtime configuration for the TranCE AI sidecar.

Everything is injected by the Qt host process (AiSidecarManager):
  - --port 0 lets uvicorn pick a free port; the chosen port is reported
    on stdout with a TRANCE_SIDECAR_READY line that the host parses.
  - the bearer token arrives via the TRANCE_AI_TOKEN environment
    variable (never on the command line, which is visible to other
    local processes).
"""

from __future__ import annotations

import os
from pathlib import Path

VERSION = "0.1.0"


class Config:
    def __init__(self, port: int, data_dir: Path) -> None:
        self.port = port
        self.data_dir = data_dir
        self.token = os.environ.get("TRANCE_AI_TOKEN", "")

    @property
    def user_data_dir(self) -> Path:
        """Per-user directory, set after the session push from the host."""
        return self.data_dir / "users" / getattr(self, "user_uuid", "")

    @property
    def web_dist_dir(self) -> Path:
        # <repo>/aisidecar/web/dist in dev; bundled next to the binary
        # when packaged.
        here = Path(__file__).resolve().parent
        return here.parent.parent / "web" / "dist"
