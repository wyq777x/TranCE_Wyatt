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

主 CMake 构建会自动准备本目录的开发环境（`TRANCE_AI_SETUP_SIDECAR`，
默认 ON）：

- `server/.venv` 缺失时用 PATH 上的 Python 3.11+ 创建并安装
  `requirements.txt`（stamp 文件 `.venv/.trance-deps-stamp` 标记完成）；
- `web/dist/index.html` 缺失或前端源码变化时自动
  `npm install && npm run build`。

两步均为自愈式：产物已存在则跳过；删掉 `.venv` / `web/dist` 即可强制
重跑。configure 阶段缺 Python 3.11+ 或 npm 会直接报错。打包构建可
`-DTRANCE_AI_SETUP_SIDECAR=OFF` 跳过（改用 PyInstaller 产物，见下文）。

需要手动操作时（例如调试 sidecar 本体）：

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
# 宿主开发回退也会优先使用 server/.venv 里的解释器（按 pyvenv.cfg 识别）
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
- P2（当前）：Native RAG（FTS5 trigram + sqlite-vec → RRF 融合）
  - `POST /api/rag/build` 构建共享语料库：dict.db 高频子集（只读
    打开）+ 内置场景表达库；embedding 走供应商 `/embeddings`，内容
    哈希缓存避免重复付费；未配置 embedding 模型时降级为纯 BM25
  - `POST /api/lookup/concept` 模糊概念反查（中文语义描述 → 英文词），
    `refine=true` 时 LLM 精选并解释
  - `POST /api/lookup/scene` 场景表达检索（business_email/academic/
    daily，中英文子串 + 语义双通道）
  - `TRANCE_AI_EMBED_MOCK=1` 用确定性哈希向量测试全管线（零 API 成本）
- P3（当前）：词网发散（mesh）
  - 离线层：内置词根/词缀静态表（44 前缀 + 34 后缀 + 64 词根），
    贪心最长匹配形态分解（unpredictable → un+pre+dict+able），
    词素 → 词族（共享词素的示例词）
  - LLM 层：同义/反义/联想词生成（结构化 JSON），结果永久缓存于
    mesh.db（词义稳定不重复付费）；无供应商时自动降级为纯离线网络
  - `POST /api/mesh/expand` 图谱（节点/边），`POST /api/mesh/explain`
    词源故事 + 记忆路径（markdown）
  - Web：ECharts 力导向图，节点按类型着色，点击查看详情、可递归发散
- P4（当前）：弱项出题（quiz）
  - 选词：弱项优先（掌握度 < 0.45 或答错 ≥ 2），自动排除最近 3 次
    出题用过的词；小词库时放宽排除保证可用
  - 生成：`POST /api/quiz/generate`（mode=cloze 完形填空 / story 情境
    故事），结构化 JSON + 严格消毒（占位符重编号、选项数校验、
    目标词覆盖校验）；画像注入使主题贴合学习者
  - LightRAG 增强层：配置 embedding 模型时启用（per-user 工作区，
    ingest 画像与既往故事，`only_need_context` 检索语境供出题延续
    主题、避免重复）；任何失败降级为直连生成，出题不依赖它
  - 判分闭环：`POST /api/quiz/submit` → 逐词回写掌握度
    （答对 +25% 递进 / 答错 ×0.55），`GET /api/quiz/history` 出题记录
- P5（当前）：MCP client 宿主
  - `mcp` 官方 SDK（pin 1.x，2.x 尚不稳）；stdio 与 streamable HTTP
    双 transport；per-user 配置 `users/<uuid>/mcp.json`（env 支持脱敏
    回显保留）
  - 连接模型：每请求连接池（规避 anyio cancel scope 跨任务关闭问
    题）；单个 server 故障不影响其余
  - 聊天集成：`use_tools` 时代理循环把工具以 `mcp__<server>__<tool>`
    前缀桥接给 OpenAI function calling，最多 5 轮工具调用，最终回答
    保持 SSE 契约（工具轮次为整段输出）
  - API：`GET/PUT /api/mcp/servers`、`POST /api/mcp/test`（连接并
    列工具）、`GET /api/mcp/tools`
  - Web：设置页管理 MCP servers（增删改/启用/测试）；聊天页
    "工具调用"开关

## 平台支持

- Windows（开发主机）：MinGW GCC 15 / Qt 6.8，全套功能验证
- Linux（Ubuntu 24.04 / Qt 6.4.2 / GCC 13）：sidecar 依赖与
  sqlite-vec、C++ configure/build、QtWebEngine 嵌入均验证通过
