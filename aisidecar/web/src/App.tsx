import { useEffect, useState } from "react";
import { fetchSessionState, type SessionState } from "./api";
import ChatPage from "./chat/ChatPage";
import LookupPage from "./lookup/LookupPage";
import MemoryPage from "./memory/MemoryPage";
import MeshPage from "./mesh/MeshPage";
import QuizPage from "./quiz/QuizPage";

type NavKey = "chat" | "mesh" | "lookup" | "quiz" | "memory";

const NAV_ITEMS: { key: NavKey; label: string; hint: string }[] = [
  { key: "chat", label: "AI 助手", hint: "" },
  { key: "mesh", label: "词网发散", hint: "" },
  { key: "lookup", label: "概念检索", hint: "" },
  { key: "quiz", label: "弱项出题", hint: "" },
  { key: "memory", label: "记忆档案", hint: "" },
];

export default function App() {
  const [active, setActive] = useState<NavKey>("chat");
  const [session, setSession] = useState<SessionState | null>(null);
  const [sessionError, setSessionError] = useState<string | null>(null);

  useEffect(() => {
    fetchSessionState()
      .then(setSession)
      .catch((err) => setSessionError(String(err)));
  }, []);

  return (
    <div className="shell">
      <aside className="sidebar">
        <div className="brand">
          <span className="brand-glow">TranCE</span> AI
        </div>
        <nav>
          {NAV_ITEMS.map((item) => (
            <button
              key={item.key}
              className={
                "nav-item" + (active === item.key ? " active" : "")
              }
              onClick={() => item.hint || setActive(item.key)}
              disabled={!!item.hint}
              title={item.hint ? `规划中（阶段 ${item.hint}）` : item.label}
            >
              {item.label}
              {item.hint && <span className="hint">{item.hint}</span>}
            </button>
          ))}
        </nav>
        <div className="session-card">
          {sessionError && <div className="error">{sessionError}</div>}
          {session && (
            <>
              <div className="session-user">
                {session.username || "未登录"}
              </div>
              <div className={session.provider_configured ? "ok" : "warn"}>
                {session.provider_configured
                  ? `模型：${session.chat_model}`
                  : "未配置 AI 供应商（请在设置中添加）"}
              </div>
            </>
          )}
        </div>
      </aside>
      <main className="content">
        {active === "chat" && (
          <ChatPage providerReady={!!session?.provider_configured} />
        )}
        {active === "memory" && (
          <MemoryPage providerReady={!!session?.provider_configured} />
        )}
        {active === "lookup" && (
          <LookupPage providerReady={!!session?.provider_configured} />
        )}
        {active === "mesh" && (
          <MeshPage providerReady={!!session?.provider_configured} />
        )}
        {active === "quiz" && (
          <QuizPage providerReady={!!session?.provider_configured} />
        )}
      </main>
    </div>
  );
}
