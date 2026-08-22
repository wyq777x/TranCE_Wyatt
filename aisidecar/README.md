# TranCE AI Sidecar

AI 模式的本地伴生服务：Python (FastAPI) 后端 + React/TS Web 前端。
由 Qt 主程序 (`AiSidecarManager`) 按需启动，仅绑定 127.0.0.1，
通过一次性 Bearer Token 鉴权（环境变量 `TRANCE_AI_TOKEN` 注入）。

```
aisidecar/
├── server/            # FastAPI sidecar (Python 3.11+)
│   ├── run.py         # 入口：--port 0 自选端口，stdout 输出 READY 行
│   ├── app/
│   │   ├── main.py    # 应用工厂：token 中间件 / 静态托管 / SPA 回退
│   │   ├── config.py  # 运行时配置（端口 / 数据目录 / web dist 路径）
│   │   ├── session.py # 内存会话（Qt 推送的用户与供应商凭据）
│   │   ├── llm.py     # OpenAI-Compatible 流式客户端
│   │   └── api/       # /api/session, /api/chat (SSE)
│   └── requirements.txt
└── web/               # Vite + React + TS 前端（构建产物由 sidecar 托管）
```

## 开发环境

```bash
# 1) 后端
cd server
python -m venv .venv
.venv/Scripts/pip install -r requirements.txt   # Linux: .venv/bin/pip

# 2) 前端
cd ../web
npm install
npm run build          # 产物 web/dist 由 sidecar 静态托管

# 3) 手动起一个 sidecar（调试用）
cd ../server
TRANCE_AI_TOKEN=dev .venv/Scripts/python run.py --port 9721 --data-dir <绝对路径>
# 前端热更新开发：cd web && npm run dev （已代理 /api 到 9721）
```

## 与 Qt 宿主的启动协议

1. 宿主生成随机 token，经环境变量 `TRANCE_AI_TOKEN` 传给子进程
   （不走命令行，避免本机其他进程可见）；
2. 宿主以 `--port 0 --data-dir <abs>` 启动，sidecar 自选空闲端口；
3. sidecar 在 stdout 打印一行
   `TRANCE_SIDECAR_READY {"port": 12345}`（flush）；
4. 宿主轮询 `GET /healthz`，成功后 `POST /api/session` 推送
   `{user_uuid, username, language, provider{base_url, api_key, ...}}`；
   API key 仅存在内存，绝不落盘、不回显；
5. Web 入口 `http://127.0.0.1:<port>/?token=<token>`，前端将 token
   存入 sessionStorage 后从地址栏移除。

## 安全模型

- 仅监听 127.0.0.1；`/api/*` 强制 Bearer Token（401）；
- 静态资源与 `/healthz` 开放（不含用户数据）；
- 供应商 API key 由 Qt 侧 OS 凭据库（Windows Credential Manager /
  Linux 文件回退）保管，仅会话期内在内存中传递。

## 发布打包（PyInstaller onedir）

```bash
cd server
.venv/Scripts/pip install pyinstaller
.venv/Scripts/pyinstaller --name aisidecar --onedir --noconfirm \
    --collect-all uvicorn --collect-all fastapi --collect-all openai \
    run.py
# 产物 dist/aisidecar/ 整目录放入主程序目录 aisidecar/ 下
# （CMake 构建的 exe 旁），并带上 ../web/dist。
```

## 路线图（对应总体方案阶段）

- P0（当前）：基础设施 + 流式聊天
- P1（当前）：掌握度记忆（learner.db + 叙事档案）
  - `POST /api/sync/snapshot` 宿主启动时推送词汇/收藏/背诵/查询快照
  - `POST /api/sync/event` 增量学习事件（答题/查词/背诵/收藏/状态）
  - 掌握度模型：答对 `m += (1-m)*0.25`，答错 `m *= 0.55`；弱项 =
    掌握度 < 0.45 或答错 ≥ 2
  - `GET/PUT /api/memory/profile` 学习者画像（narrative）查看/编辑；
    `POST /api/memory/consolidate` LLM 固化画像
  - `/api/chat` 默认注入画像 + 弱项词上下文（`use_memory` 可关）
- P2：Native RAG（FTS5 + sqlite-vec 混合检索：概念反查/场景检索）
- P3：词根/词缀/近反义网状发散（图谱 + ECharts）
- P4：LightRAG 弱项故事/完形出题
- P5：MCP client 宿主
