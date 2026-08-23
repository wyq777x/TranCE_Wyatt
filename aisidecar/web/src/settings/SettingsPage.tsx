import { useCallback, useEffect, useState } from "react";
import {
  fetchMcpServers,
  saveMcpServers,
  testMcpServer,
  type McpServerConfig,
  type McpTestResult,
} from "../api";

const EMPTY: McpServerConfig = {
  name: "",
  transport: "stdio",
  command: "",
  args: [],
  env: {},
  url: "",
  enabled: true,
};

export default function SettingsPage() {
  const [servers, setServers] = useState<McpServerConfig[]>([]);
  const [editing, setEditing] = useState<McpServerConfig | null>(null);
  const [argsText, setArgsText] = useState("");
  const [envText, setEnvText] = useState("");
  const [tests, setTests] = useState<Record<string, McpTestResult>>({});
  const [message, setMessage] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  const reload = useCallback(async () => {
    try {
      setServers(await fetchMcpServers());
    } catch (err) {
      setMessage(String(err));
    }
  }, []);

  useEffect(() => {
    void reload();
  }, [reload]);

  function startEdit(server: McpServerConfig) {
    setEditing({ ...server });
    setArgsText((server.args ?? []).join(" "));
    setEnvText(
      Object.entries(server.env ?? {})
        .map(([k, v]) => `${k}=${v}`)
        .join("\n"),
    );
    setMessage(null);
  }

  function parseEnv(text: string): Record<string, string> {
    const env: Record<string, string> = {};

    for (const line of text.split("\n")) {
      const trimmed = line.trim();

      if (!trimmed) continue;

      const eq = trimmed.indexOf("=");

      if (eq > 0) {
        env[trimmed.slice(0, eq)] = trimmed.slice(eq + 1);
      }
    }

    return env;
  }

  async function onSave() {
    if (!editing) return;

    setBusy(true);
    setMessage(null);

    try {
      const updated = {
        ...editing,
        name: editing.name.trim(),
        command: editing.command.trim(),
        url: editing.url.trim(),
        args: argsText.split(/\s+/).filter(Boolean),
        env: parseEnv(envText),
      };

      if (!updated.name) throw new Error("名称不能为空");

      const next = servers.some((s) => s.name === updated.name)
        ? servers.map((s) => (s.name === updated.name ? updated : s))
        : [...servers, updated];

      const saved = await saveMcpServers(next);
      setServers(saved);
      setEditing(null);
      setMessage("已保存");
    } catch (err) {
      setMessage(String(err));
    } finally {
      setBusy(false);
    }
  }

  async function onRemove(name: string) {
    setBusy(true);

    try {
      const saved = await saveMcpServers(
        servers.filter((s) => s.name !== name),
      );
      setServers(saved);
      setMessage(`已删除 ${name}`);
    } catch (err) {
      setMessage(String(err));
    } finally {
      setBusy(false);
    }
  }

  async function onToggle(server: McpServerConfig) {
    setBusy(true);

    try {
      const saved = await saveMcpServers(
        servers.map((s) =>
          s.name === server.name ? { ...s, enabled: !s.enabled } : s,
        ),
      );
      setServers(saved);
    } catch (err) {
      setMessage(String(err));
    } finally {
      setBusy(false);
    }
  }

  async function onTest() {
    if (!editing) return;

    setBusy(true);

    try {
      const result = await testMcpServer({
        ...editing,
        args: argsText.split(/\s+/).filter(Boolean),
        env: parseEnv(envText),
      });
      setTests((prev) => ({ ...prev, [editing.name || "test"]: result }));
    } finally {
      setBusy(false);
    }
  }

  const testResult = editing ? tests[editing.name || "test"] : undefined;

  return (
    <div className="settings-page">
      <header className="memory-header">
        <h2>设置</h2>
        <button className="primary-btn" onClick={() => startEdit(EMPTY)}>
          添加 MCP 服务器
        </button>
      </header>

      {message && <div className="warn banner">{message}</div>}

      <section className="memory-section">
        <h3>
          MCP 服务器
          <span className="dim" style={{ fontWeight: 400 }}>
            {" "}
            · 本地 stdio 命令或远程 HTTP，工具会桥接进 AI 助手
          </span>
        </h3>

        {servers.length === 0 && (
          <p className="placeholder">
            尚未配置。例如接入网页搜索或词典 MCP 服务器，AI
            助手即可调用外部工具。
          </p>
        )}

        {servers.map((server) => (
          <div key={server.name} className="mcp-row">
            <label className="memory-toggle">
              <input
                type="checkbox"
                checked={server.enabled}
                disabled={busy}
                onChange={() => void onToggle(server)}
              />
            </label>
            <div className="mcp-info">
              <b>{server.name}</b>
              <span className="dim">
                {" "}
                {server.transport === "http"
                  ? server.url
                  : [server.command, ...(server.args ?? [])].join(" ")}
              </span>
            </div>
            <button className="ghost-btn" onClick={() => startEdit(server)}>
              编辑
            </button>
            <button
              className="ghost-btn"
              disabled={busy}
              onClick={() => void onRemove(server.name)}
            >
              删除
            </button>
          </div>
        ))}
      </section>

      {editing && (
        <section className="memory-section">
          <h3>{servers.some((s) => s.name === editing.name) ? "编辑" : "新增"} MCP 服务器</h3>

          <div className="mcp-form">
            <label>
              名称
              <input
                className="search-input"
                value={editing.name}
                onChange={(e) =>
                  setEditing({ ...editing, name: e.target.value })
                }
                placeholder="如 web-search"
              />
            </label>

            <label>
              传输方式
              <select
                value={editing.transport}
                onChange={(e) =>
                  setEditing({
                    ...editing,
                    transport: e.target.value as "stdio" | "http",
                  })
                }
              >
                <option value="stdio">stdio（本地命令）</option>
                <option value="http">HTTP（远程服务器）</option>
              </select>
            </label>

            {editing.transport === "stdio" ? (
              <>
                <label>
                  命令
                  <input
                    className="search-input"
                    value={editing.command}
                    onChange={(e) =>
                      setEditing({ ...editing, command: e.target.value })
                    }
                    placeholder="如 npx / uvx / python"
                  />
                </label>
                <label>
                  参数（空格分隔）
                  <input
                    className="search-input"
                    value={argsText}
                    onChange={(e) => setArgsText(e.target.value)}
                    placeholder="如 -y @mcp/server-everything"
                  />
                </label>
              </>
            ) : (
              <label>
                服务器 URL
                <input
                  className="search-input"
                  value={editing.url}
                  onChange={(e) =>
                    setEditing({ ...editing, url: e.target.value })
                  }
                  placeholder="https://example.com/mcp"
                />
              </label>
            )}

            <label>
              环境变量（每行 KEY=VALUE，如 API 密钥）
              <textarea
                className="narrative-editor"
                value={envText}
                onChange={(e) => setEnvText(e.target.value)}
                rows={3}
              />
            </label>

            <div className="memory-actions">
              <button
                className="ghost-btn"
                disabled={busy}
                onClick={() => void onTest()}
              >
                {busy ? "测试中…" : "测试连接"}
              </button>
              <button className="primary-btn" disabled={busy} onClick={() => void onSave()}>
                保存
              </button>
              <button className="ghost-btn" onClick={() => setEditing(null)}>
                取消
              </button>
            </div>

            {testResult &&
              (testResult.ok ? (
                <div className="ok banner">
                  连接成功，工具：{testResult.tools?.map((t) => t.name).join(", ")}
                </div>
              ) : (
                <div className="error banner">连接失败：{testResult.error}</div>
              ))}
          </div>
        </section>
      )}
    </div>
  );
}
