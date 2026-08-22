import { useEffect, useRef, useState } from "react";
import ReactMarkdown from "react-markdown";
import { streamChat, type ChatMessage } from "../api";

interface UiMessage extends ChatMessage {
  streaming?: boolean;
  error?: boolean;
}

const SYSTEM_PROMPT: ChatMessage = {
  role: "system",
  content:
    "你是 TranCE 的 AI 语言学习助手，帮助中国用户学习英语。" +
    "回答使用简体中文，语言点讲解保留必要的英文原文。" +
    "当前阶段为基础聊天模式，后续将接入用户词汇画像与 RAG 知识库。",
};

export default function ChatPage({
  providerReady,
}: {
  providerReady: boolean;
}) {
  const [messages, setMessages] = useState<UiMessage[]>([
    {
      role: "assistant",
      content: "你好！我是 TranCE AI 助手，问我任何英语学习问题吧。",
    },
  ]);
  const [input, setInput] = useState("");
  const [busy, setBusy] = useState(false);
  const [useMemory, setUseMemory] = useState(true);
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    scrollRef.current?.scrollTo({
      top: scrollRef.current.scrollHeight,
    });
  }, [messages]);

  async function send() {
    const text = input.trim();
    if (!text || busy) return;

    if (!providerReady) {
      setMessages((prev) => [
        ...prev,
        { role: "user", content: text },
        {
          role: "assistant",
          error: true,
          content: "尚未配置 AI 供应商。请在 TranCE 设置中添加供应商并激活。",
        },
      ]);
      setInput("");
      return;
    }

    const history: ChatMessage[] = messages
      .filter((m) => !m.error)
      .map(({ role, content }) => ({ role, content }));

    const nextMessages: UiMessage[] = [
      ...messages,
      { role: "user", content: text },
      { role: "assistant", content: "", streaming: true },
    ];
    setMessages(nextMessages);
    setInput("");
    setBusy(true);

    const assistantIndex = nextMessages.length - 1;

    try {
      await streamChat(
        [SYSTEM_PROMPT, ...history, { role: "user", content: text }],
        (delta) => {
          setMessages((prev) => {
            const copy = [...prev];
            copy[assistantIndex] = {
              ...copy[assistantIndex],
              content: copy[assistantIndex].content + delta,
            };
            return copy;
          });
        },
        useMemory,
      );
      setMessages((prev) => {
        const copy = [...prev];
        copy[assistantIndex] = { ...copy[assistantIndex], streaming: false };
        return copy;
      });
    } catch (err) {
      setMessages((prev) => {
        const copy = [...prev];
        copy[assistantIndex] = {
          role: "assistant",
          error: true,
          streaming: false,
          content: `出错了：${String(err)}`,
        };
        return copy;
      });
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="chat-page">
      <div className="chat-scroll" ref={scrollRef}>
        {messages.map((message, index) => (
          <div
            key={index}
            className={`bubble ${message.role} ${
              message.error ? "error" : ""
            }`}
          >
            {message.role === "assistant" ? (
              <div className="markdown">
                <ReactMarkdown>{message.content}</ReactMarkdown>
                {message.streaming && (
                  <span className="cursor">▍</span>
                )}
              </div>
            ) : (
              message.content
            )}
          </div>
        ))}
      </div>
      <div className="chat-input-row">
        <textarea
          value={input}
          placeholder="输入问题，Enter 发送，Shift+Enter 换行"
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter" && !e.shiftKey) {
              e.preventDefault();
              void send();
            }
          }}
          rows={3}
        />
        <div className="send-column">
          <label className="memory-toggle" title="将学习者画像与弱项词注入上下文，使回答个性化">
            <input
              type="checkbox"
              checked={useMemory}
              onChange={(e) => setUseMemory(e.target.checked)}
            />
            记忆增强
          </label>
          <button
            className="send-btn"
            onClick={() => void send()}
            disabled={busy}
          >
            {busy ? "生成中…" : "发送"}
          </button>
        </div>
      </div>
    </div>
  );
}
