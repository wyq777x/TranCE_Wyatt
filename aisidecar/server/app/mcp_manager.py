"""MCP client host: configuration, connections and tool bridging.

The sidecar acts as an MCP *client*: it connects to user-configured MCP
servers (stdio for local commands, streamable HTTP for remote) and
bridges their tools into the chat agent loop via OpenAI function
calling.

Connection model: connections live for the duration of one chat request
(a per-request pool). Long-lived sessions would need careful task
ownership (anyio cancel scopes must be exited in the task that entered
them), and per-request pools keep the lifecycle trivial at a latency
cost that is small next to LLM round-trips.

Configuration (per user, plaintext - treat the data dir as private):
    data_dir/users/<uuid>/mcp.json
    {"servers": [{"name", "transport": "stdio"|"http", "command",
                  "args": [], "env": {}, "url", "enabled"}]}
"""

from __future__ import annotations

import contextlib
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, AsyncIterator, Optional

from .config import Config
from .session import get_session

TOOL_PREFIX = "mcp__"


@dataclass
class McpServerConfig:
    name: str
    transport: str = "stdio"  # stdio | http
    command: str = ""
    args: list[str] = field(default_factory=list)
    env: dict[str, str] = field(default_factory=dict)
    url: str = ""
    enabled: bool = True

    def to_dict(self) -> dict:
        return {
            "name": self.name,
            "transport": self.transport,
            "command": self.command,
            "args": self.args,
            "env": self.env,
            "url": self.url,
            "enabled": self.enabled,
        }

    @staticmethod
    def from_dict(data: dict) -> "McpServerConfig":
        return McpServerConfig(
            name=str(data.get("name", "")).strip(),
            transport="http" if data.get("transport") == "http" else "stdio",
            command=str(data.get("command", "")),
            args=[str(a) for a in data.get("args", [])],
            env={
                str(k): str(v) for k, v in (data.get("env") or {}).items()
            },
            url=str(data.get("url", "")),
            enabled=bool(data.get("enabled", True)),
        )

    def validate(self) -> Optional[str]:
        if not self.name:
            return "name is required"

        if self.transport == "stdio" and not self.command:
            return "command is required for stdio transport"

        if self.transport == "http" and not self.url.startswith("http"):
            return "url must start with http(s):// for http transport"

        return None


def _redact(server: McpServerConfig) -> dict:
    data = server.to_dict()

    if data["env"]:
        data["env_keys"] = list(data["env"])
        data["env"] = {}

    return data


class McpManager:
    """Config persistence + one-shot connection helpers."""

    def __init__(self, config: Config) -> None:
        self.config = config

    @property
    def config_path(self) -> Path:
        return (
            self.config.data_dir
            / "users"
            / get_session().user_uuid
            / "mcp.json"
        )

    def load_servers(self) -> list[McpServerConfig]:
        try:
            data = json.loads(self.config_path.read_text("utf-8"))
            return [McpServerConfig.from_dict(s) for s in data.get("servers", [])]
        except (OSError, json.JSONDecodeError):
            return []

    def save_servers(self, servers: list[McpServerConfig]) -> None:
        self.config_path.parent.mkdir(parents=True, exist_ok=True)
        self.config_path.write_text(
            json.dumps(
                {"servers": [s.to_dict() for s in servers]},
                ensure_ascii=False,
                indent=2,
            ),
            "utf-8",
        )

    def public_servers(self) -> list[dict]:
        return [_redact(s) for s in self.load_servers()]

    # ---------- one-shot connections ----------

    @contextlib.asynccontextmanager
    async def connect(
        self, server: McpServerConfig
    ) -> AsyncIterator["ClientSession"]:  # noqa: F821
        """Yield an initialized ClientSession; closed on exit."""
        from mcp import ClientSession, StdioServerParameters
        from mcp.client.stdio import stdio_client

        if server.transport == "http":
            from mcp.client.streamable_http import streamablehttp_client

            async with streamablehttp_client(server.url) as (
                read,
                write,
                _,
            ):
                async with ClientSession(read, write) as session:
                    await session.initialize()
                    yield session
        else:
            params = StdioServerParameters(
                command=server.command,
                args=server.args,
                env={**server.env} if server.env else None,
            )
            async with stdio_client(params) as (read, write):
                async with ClientSession(read, write) as session:
                    await session.initialize()
                    yield session

    async def test_server(
        self, server: McpServerConfig
    ) -> dict:
        error = server.validate()

        if error:
            return {"ok": False, "error": error}

        try:
            async with self.connect(server) as session:
                result = await session.list_tools()
                return {
                    "ok": True,
                    "tools": [
                        {
                            "name": t.name,
                            "description": t.description or "",
                        }
                        for t in result.tools
                    ],
                }
        except Exception as exc:
            return {"ok": False, "error": f"{type(exc).__name__}: {exc}"}


class McpPool:
    """All enabled servers connected for the duration of one request."""

    def __init__(self, manager: McpManager) -> None:
        self.manager = manager
        self._sessions: dict[str, Any] = {}
        self._tools: dict[str, tuple[str, str]] = {}  # prefixed -> (server, tool)
        self._tool_specs: list[dict] = []

    async def __aenter__(self) -> "McpPool":
        for server in self.manager.load_servers():
            if not server.enabled:
                continue

            try:
                cm = self.manager.connect(server)
                session = await cm.__aenter__()
                self._sessions[server.name] = (cm, session)
                result = await session.list_tools()

                for tool in result.tools:
                    prefixed = f"{TOOL_PREFIX}{server.name}__{tool.name}"
                    self._tools[prefixed] = (server.name, tool.name)
                    self._tool_specs.append({
                        "type": "function",
                        "function": {
                            "name": prefixed,
                            "description": (
                                f"[{server.name}] " + (tool.description or "")
                            )[:512],
                            "parameters": tool.inputSchema
                            or {"type": "object", "properties": {}},
                        },
                    })
            except Exception:
                # one broken server must not take down the whole pool
                continue

        return self

    async def __aexit__(self, *exc_info: object) -> None:
        for cm, _session in reversed(list(self._sessions.values())):
            with contextlib.suppress(Exception):
                await cm.__aexit__(*exc_info)  # type: ignore[arg-type]

        self._sessions.clear()
        self._tools.clear()
        self._tool_specs.clear()

    @property
    def has_tools(self) -> bool:
        return bool(self._tool_specs)

    async def openai_tools(self) -> list[dict]:
        """Tool specs in OpenAI function-calling form (cached from
        connection time)."""
        return self._tool_specs

    async def call(self, prefixed_name: str, arguments: dict) -> str:
        mapping = self._tools.get(prefixed_name)

        if mapping is None:
            return f"error: unknown tool {prefixed_name}"

        server_name, tool_name = mapping
        _cm, session = self._sessions[server_name]

        try:
            result = await session.call_tool(tool_name, arguments)

            parts = [
                getattr(c, "text", "")
                for c in result.content
                if getattr(c, "text", "")
            ]

            text = "\n".join(parts) or "(empty tool result)"

            if getattr(result, "isError", False):
                return f"tool error: {text}"

            return text
        except Exception as exc:
            return f"error calling {tool_name}: {type(exc).__name__}: {exc}"
