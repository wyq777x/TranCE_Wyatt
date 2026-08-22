import { useCallback, useEffect, useState } from "react";
import ReactMarkdown from "react-markdown";
import {
  consolidateMemory,
  fetchMemoryProfile,
  saveNarrative,
  type MemoryProfile,
} from "../api";

export default function MemoryPage({
  providerReady,
}: {
  providerReady: boolean;
}) {
  const [profile, setProfile] = useState<MemoryProfile | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [editing, setEditing] = useState(false);
  const [draft, setDraft] = useState("");
  const [busy, setBusy] = useState<string | null>(null);

  const reload = useCallback(async () => {
    try {
      setProfile(await fetchMemoryProfile());
      setError(null);
    } catch (err) {
      setError(String(err));
    }
  }, []);

  useEffect(() => {
    void reload();
  }, [reload]);

  async function onSave() {
    setBusy("save");
    try {
      await saveNarrative(draft);
      setEditing(false);
      await reload();
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(null);
    }
  }

  async function onConsolidate() {
    setBusy("consolidate");
    try {
      await consolidateMemory();
      await reload();
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(null);
    }
  }

  return (
    <div className="memory-page">
      <header className="memory-header">
        <h2>记忆档案</h2>
        <div className="memory-actions">
          <button
            className="ghost-btn"
            disabled={!!busy}
            onClick={() => void onConsolidate()}
            title={
              providerReady
                ? "让 AI 根据近期学习行为总结/更新画像"
                : "需要先配置 AI 供应商"
            }
          >
            {busy === "consolidate" ? "AI 总结中…" : "AI 固化画像"}
          </button>
          {editing ? (
            <>
              <button className="ghost-btn" onClick={() => setEditing(false)}>
                取消
              </button>
              <button
                className="primary-btn"
                disabled={busy === "save"}
                onClick={() => void onSave()}
              >
                {busy === "save" ? "保存中…" : "保存"}
              </button>
            </>
          ) : (
            <button
              className="ghost-btn"
              onClick={() => {
                setDraft(profile?.narrative ?? "");
                setEditing(true);
              }}
            >
              编辑
            </button>
          )}
        </div>
      </header>

      {error && <div className="error banner">{error}</div>}

      {profile && (
        <>
          <div className="stat-cards">
            <div className="stat-card">
              <div className="stat-value">{profile.stats.total_words}</div>
              <div className="stat-label">收录词汇</div>
            </div>
            <div className="stat-card">
              <div className="stat-value ok">
                {profile.stats.strong_words}
              </div>
              <div className="stat-label">高掌握 (≥0.8)</div>
            </div>
            <div className="stat-card">
              <div className="stat-value warn">
                {profile.stats.weak_words}
              </div>
              <div className="stat-label">弱项词</div>
            </div>
            <div className="stat-card">
              <div className="stat-value">{profile.stats.events}</div>
              <div className="stat-label">学习事件</div>
            </div>
          </div>

          <section className="memory-section">
            <h3>学习者画像 {profile.updated_at && `· 更新于 ${profile.updated_at}`}</h3>
            {editing ? (
              <textarea
                className="narrative-editor"
                value={draft}
                onChange={(e) => setDraft(e.target.value)}
                rows={10}
              />
            ) : profile.narrative ? (
              <div className="markdown narrative-view">
                <ReactMarkdown>{profile.narrative}</ReactMarkdown>
              </div>
            ) : (
              <p className="placeholder">
                暂无画像。答题、查词、背诵后点击「AI 固化画像」生成，
                或手动编辑写入你对学习的自述——所有 AI 功能都会参考这份画像。
              </p>
            )}
          </section>

          <section className="memory-section">
            <h3>弱项词</h3>
            {profile.weak_words.length === 0 ? (
              <p className="placeholder">
                暂无弱项记录（掌握度 &lt; 0.45 或答错 ≥ 2 次的词会出现在这里）。
              </p>
            ) : (
              <table className="weak-table">
                <thead>
                  <tr>
                    <th>词</th>
                    <th>掌握度</th>
                    <th>答对/答错</th>
                    <th>接触</th>
                  </tr>
                </thead>
                <tbody>
                  {profile.weak_words.map((w) => (
                    <tr key={w.word}>
                      <td className="word-cell">{w.word}</td>
                      <td>
                        <div className="mastery-bar">
                          <div
                            className="mastery-fill"
                            style={{
                              width: `${Math.round(w.mastery * 100)}%`,
                            }}
                          />
                        </div>
                        <span className="mastery-num">{w.mastery}</span>
                      </td>
                      <td>
                        <span className="ok">{w.correct_count}</span> /{" "}
                        <span className="bad">{w.wrong_count}</span>
                      </td>
                      <td className="dim">
                        查{w.lookups} · 背{w.recites}
                        {w.favorite ? " · ★" : ""}
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            )}
          </section>
        </>
      )}
    </div>
  );
}
