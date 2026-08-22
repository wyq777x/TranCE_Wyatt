"""Sidecar entry point invoked by the Qt host (AiSidecarManager).

Startup handshake:
  argv:   run.py --port 0 --data-dir <dir>
  env:    TRANCE_AI_TOKEN=<one-time bearer token>
  stdout: one line  TRANCE_SIDECAR_READY {"port": <actual-port>}
          (printed with flush - the host parses it to learn the port)

Development: TRANCE_AI_TOKEN=dev python run.py --port 9721 --data-dir
./dev-data works for manual testing outside the Qt app.
"""

from __future__ import annotations

import argparse
import asyncio
import json
import sys
from pathlib import Path

import uvicorn

from app.config import Config
from app.main import create_app


async def serve_and_report(server: uvicorn.Server, requested_port: int) -> None:
    serve_task = asyncio.ensure_future(server.serve())

    # uvicorn binds its sockets inside serve(); wait until startup finished
    # so the actually-bound port can be reported to the host.
    while not server.started:
        await asyncio.sleep(0.05)

    port = requested_port

    if server.servers:
        port = server.servers[0].sockets[0].getsockname()[1]

    print(
        "TRANCE_SIDECAR_READY " + json.dumps({"port": port}),
        flush=True,
    )

    await serve_task


def main() -> int:
    parser = argparse.ArgumentParser(description="TranCE AI sidecar")
    parser.add_argument("--port", type=int, default=0,
                        help="port to bind (0 = pick a free port)")
    parser.add_argument("--data-dir", type=str, required=True,
                        help="directory for sidecar-owned data")
    args = parser.parse_args()

    if not Path(args.data_dir).is_absolute():
        print("TRANCE_SIDECAR_ERROR --data-dir must be absolute", flush=True)
        return 2

    data_dir = Path(args.data_dir).expanduser().resolve()
    data_dir.mkdir(parents=True, exist_ok=True)

    config = Config(port=args.port, data_dir=data_dir)

    if not config.token:
        print("TRANCE_SIDECAR_ERROR missing TRANCE_AI_TOKEN", flush=True)
        return 2

    app = create_app(config)

    server = uvicorn.Server(
        uvicorn.Config(
            app,
            host="127.0.0.1",
            port=args.port,
            log_level="warning",
            access_log=False,
        )
    )

    asyncio.run(serve_and_report(server, args.port))

    return 0


if __name__ == "__main__":
    sys.exit(main())
