import { useCallback, useEffect, useMemo, useState } from "react";
import {
  fetchQuizHistory,
  generateQuiz,
  submitQuiz,
  type Quiz,
  type QuizHistoryEntry,
  type QuizScore,
} from "../api";

function ClozePassage({
  quiz,
  answers,
  score,
  onPick,
}: {
  quiz: Quiz;
  answers: Record<string, string>;
  score: QuizScore | null;
  onPick: (index: number, option: string) => void;
}) {
  const parts = useMemo(
    () => quiz.passage.split(/(\{\d+\})/g).filter(Boolean),
    [quiz.passage],
  );

  return (
    <p className="quiz-passage">
      {parts.map((part, i) => {
        const match = part.match(/^\{(\d+)\}$/);

        if (!match) return <span key={i}>{part}</span>;

        const index = Number(match[1]);
        const chosen = answers[String(index)];
        const result = score?.results.find((r) => r.index === index);

        return (
          <span
            key={i}
            className={
              "cloze-slot" +
              (result ? (result.correct ? " correct" : " wrong") : "") +
              (chosen && !score ? " filled" : "")
            }
          >
            {chosen || `(${index})`}
            {result && !result.correct && (
              <span className="cloze-answer"> → {result.word}</span>
            )}
            <span className="cloze-options">
              {(quiz.items.find((it) => it.index === index)?.options ?? []).map(
                (option) => (
                  <button
                    key={option}
                    className="option-btn"
                    disabled={!!score}
                    onClick={() => onPick(index, option)}
                  >
                    {option}
                  </button>
                ),
              )}
            </span>
          </span>
        );
      })}
    </p>
  );
}

export default function QuizPage({
  providerReady,
}: {
  providerReady: boolean;
}) {
  const [mode, setMode] = useState<"cloze" | "story">("cloze");
  const [count, setCount] = useState(4);
  const [manualWords, setManualWords] = useState("");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [quizId, setQuizId] = useState<number | null>(null);
  const [quiz, setQuiz] = useState<Quiz | null>(null);
  const [ragUsed, setRagUsed] = useState(false);
  const [answers, setAnswers] = useState<Record<string, string>>({});
  const [score, setScore] = useState<QuizScore | null>(null);
  const [history, setHistory] = useState<QuizHistoryEntry[]>([]);

  const refreshHistory = useCallback(async () => {
    try {
      setHistory(await fetchQuizHistory());
    } catch {
      /* history is best-effort */
    }
  }, []);

  useEffect(() => {
    void refreshHistory();
  }, [refreshHistory]);

  async function onGenerate() {
    if (busy || !providerReady) return;

    setBusy(true);
    setError(null);
    setQuiz(null);
    setScore(null);
    setAnswers({});

    try {
      const words = manualWords
        .split(/[,，\s]+/)
        .map((w) => w.trim())
        .filter(Boolean);
      const result = await generateQuiz(mode, count, words);
      setQuizId(result.quiz_id);
      setQuiz(result.quiz);
      setRagUsed(result.rag_used);
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(false);
    }
  }

  function onPick(index: number, option: string) {
    setAnswers((prev) => ({ ...prev, [String(index)]: option }));
  }

  async function onSubmit() {
    if (!quizId || !quiz || score) return;

    const missing = quiz.items.filter(
      (it) => !answers[String(it.index)],
    ).length;

    if (missing > 0) {
      setError(`还有 ${missing} 个空未作答`);
      return;
    }

    setBusy(true);
    setError(null);

    try {
      setScore(await submitQuiz(quizId, answers));
      await refreshHistory();
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="quiz-page">
      <div className="quiz-toolbar">
        <div className="tab-row">
          <button
            className={"tab-btn" + (mode === "cloze" ? " active" : "")}
            onClick={() => setMode("cloze")}
          >
            完形填空
          </button>
          <button
            className={"tab-btn" + (mode === "story" ? " active" : "")}
            onClick={() => setMode("story")}
          >
            情境故事
          </button>
        </div>
        <label className="quiz-count">
          词数
          <select
            value={count}
            onChange={(e) => setCount(Number(e.target.value))}
          >
            {[2, 3, 4, 5, 6].map((n) => (
              <option key={n} value={n}>
                {n}
              </option>
            ))}
          </select>
        </label>
        <input
          className="search-input quiz-manual"
          placeholder="指定词（可选，逗号分隔；留空自动选弱项）"
          value={manualWords}
          onChange={(e) => setManualWords(e.target.value)}
        />
        <button
          className="primary-btn"
          disabled={busy || !providerReady}
          onClick={() => void onGenerate()}
          title={
            providerReady ? "" : "需要先配置 AI 供应商"
          }
        >
          {busy ? "生成中…" : mode === "cloze" ? "出一套题" : "写一篇故事"}
        </button>
      </div>

      {error && <div className="error banner">{error}</div>}
      {!providerReady && (
        <div className="warn banner">
          出题需要 AI 供应商。当前会基于弱项词自动选题，生成由你配置的
          LLM 完成。
        </div>
      )}

      {quiz && (
        <div className="quiz-card">
          <div className="quiz-title-row">
            <h3>{quiz.title}</h3>
            <div className="quiz-meta">
              {ragUsed && (
                <span className="tag tag-vec" title="LightRAG 语境增强">
                  记忆延续
                </span>
              )}
              {quiz.target_words.map((w) => (
                <span key={w} className="tag tag-root">
                  {w}
                </span>
              ))}
            </div>
          </div>

          {quiz.type === "cloze" ? (
            <>
              <ClozePassage
                quiz={quiz}
                answers={answers}
                score={score}
                onPick={onPick}
              />
              {!score && (
                <button
                  className="primary-btn"
                  disabled={busy}
                  onClick={() => void onSubmit()}
                >
                  提交判分
                </button>
              )}
              {score && (
                <div className="score-banner">
                  <b>
                    {score.correct_count} / {score.total}
                  </b>
                  <span> 已回写掌握度 · </span>
                  <button
                    className="ghost-btn"
                    onClick={() => void onGenerate()}
                  >
                    再来一套
                  </button>
                </div>
              )}
              {score &&
                score.results
                  .filter((r) => !r.correct)
                  .map((r) => (
                    <div key={r.index} className="explain-row wrong">
                      <b>{r.word}</b>：你选了 {r.chosen}。{r.explanation}
                    </div>
                  ))}
            </>
          ) : (
            <>
              <p className="quiz-passage story">
                {quiz.passage.split(/\b/).map((token, i) => {
                  const clean = token.toLowerCase().replace(/[^a-z-]/g, "");
                  return quiz.target_words.includes(clean) ? (
                    <mark key={i} className="story-highlight">
                      {token}
                    </mark>
                  ) : (
                    <span key={i}>{token}</span>
                  );
                })}
              </p>
              <button
                className="ghost-btn"
                disabled={busy}
                onClick={() => void onGenerate()}
              >
                再写一篇
              </button>
            </>
          )}

          {quiz.glossary.length > 0 && (
            <div className="morpheme-list">
              <h4>词表</h4>
              {quiz.glossary.map((g) => (
                <div key={g.word} className="morpheme-row">
                  <span className="morpheme-tag tag-root">{g.word}</span>
                  <span className="dim">{g.meaning}</span>
                </div>
              ))}
            </div>
          )}
        </div>
      )}

      {history.length > 0 && (
        <section className="memory-section">
          <h3>出题记录</h3>
          {history.map((h) => (
            <div key={h.quiz_id} className="history-row">
              <span className="dim">#{h.quiz_id}</span>
              <span>{h.mode === "cloze" ? "完形" : "故事"}</span>
              <span className="dim">{h.created_at.slice(5, 16)}</span>
              <span className="quiz-words">
                {h.words.slice(0, 6).join(", ")}
              </span>
              {h.submitted && h.score ? (
                <span
                  className={
                    h.score.correct_count === h.score.total ? "ok" : "warn"
                  }
                >
                  {h.score.correct_count}/{h.score.total}
                </span>
              ) : (
                <span className="dim">未提交</span>
              )}
            </div>
          ))}
        </section>
      )}
    </div>
  );
}
