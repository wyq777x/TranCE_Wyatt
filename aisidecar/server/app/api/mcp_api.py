"""MCP configuration endpoints.

GET  /api/mcp/servers   saved server configs (env values redacted)
PUT  /api/mcp/servers   replace the whole config
POST /api/mcp/test      connect once and list tools (no persistence)
GET  /api/mcp/tools     tools of all enabled servers (connects each)
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Request
from pydantic import BaseModel

from ..mcp_manager import McpManager, McpServerConfig

router = APIRouter()


def _manager(request: Request) -> McpManager:
    manager = getattr(request.app.state, "mcp_manager", None)

    if manager is None:
        manager = McpManager(request.app.state.config)
        request.app.state.mcp_manager = manager

    return manager


class ServerPayload(BaseModel):
    name: str
    transport: str = "stdio"
    command: str = ""
    args: list[str] = []
    env: dict[str, str] = {}
    url: str = ""
    enabled: bool = True


class ServersPayload(BaseModel):
    servers: list[ServerPayload]


@router.get("/api/mcp/servers")
def get_servers(request: Request) -> dict:
    return {"servers": _manager(request).public_servers()}


@router.put("/api/mcp/servers")
async def put_servers(payload: ServersPayload, request: Request) -> dict:
    manager = _manager(request)
    servers: list[McpServerConfig] = []
    names: set[str] = set()

    for item in payload.servers:
        config = McpServerConfig(
            name=item.name.strip(),
            transport=item.transport,
            command=item.command.strip(),
            args=item.args,
            env=item.env,
            url=item.url.strip(),
            enabled=item.enabled,
        )
        error = config.validate()

        if error:
            raise HTTPException(status_code=400, detail=f"{config.name}: {error}")

        if config.name in names:
            raise HTTPException(
                status_code=400, detail=f"duplicate server name: {config.name}"
            )

        names.add(config.name)
        servers.append(config)

    # keep env values of entries the client redacted back (env empty +
    # env_keys present means "unchanged")
    old = {s.name: s for s in manager.load_servers()}

    for config in servers:
        if not config.env:
            old_server = old.get(config.name)

            if old_server is not None:
                config.env = old_server.env

    manager.save_servers(servers)
    return {"ok": True, "servers": manager.public_servers()}


@router.post("/api/mcp/test")
async def test_server(payload: ServerPayload, request: Request) -> dict:
    config = McpServerConfig(
        name=payload.name.strip() or "test",
        transport=payload.transport,
        command=payload.command.strip(),
        args=payload.args,
        env=payload.env,
        url=payload.url.strip(),
        enabled=payload.enabled,
    )
    return await _manager(request).test_server(config)


@router.get("/api/mcp/tools")
async def list_tools(request: Request) -> dict:
    from ..mcp_manager import McpPool

    pool = McpPool(_manager(request))

    async with pool:
        specs = await pool.openai_tools()

        return {
            "tools": [
                {
                    "name": s["function"]["name"],
                    "description": s["function"]["description"],
                }
                for s in specs
            ]
        }
