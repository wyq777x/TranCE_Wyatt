import { useCallback, useEffect, useRef, useState } from "react";
import ReactMarkdown from "react-markdown";
import {
  buildRag,
  fetchRagStatus,
  lookupConcept,
  lookupScene,
  type ConceptResult,
  type LookupHit,
  type RagStatus,
  type SceneResult,
} from "../api";

const SCENES = [
  { key: "business_email", label: "商务邮件" },
  { key: "academic", label: "学术写作" },
  { key: "daily", label: "日常口语" },
];

function SourceTag({ sources }: { sources: string[] }) {
  return (
    <span className="source-tags">
      {sources.includes("bm25") && (
        <span className="tag tag-bm25" title="关键词/子串匹配 (FTS5 BM25)">
          BM25
        </span>
      )}
      {sources.includes("vec") && (
        <span className="tag tag-vec" title="语义向量匹配 (KNN)">
          语义
        </span>
      )}
      {sources.length === 0 && <span className="tag tag-list">全部</span>}
    </span>
  );
}

function HitCard({ hit }: { hit: LookupHit }) {
  return (
    <div className="hit-card">
      <div className="hit-main">
        <span className="hit-word">{hit.word}</span>
        {hit.note && <span className="hit-pos">{hit.note}</span>}
        {hit.frequency > 0 && (
          <span className="hit-freq" title="词频排名">
            #{hit.frequency}
          </span>
        )}
        <SourceTag sources={hit.sources} />
      </div>
      <div className="hit-translation">{hit.translation}</div>
      {hit.scene && <div className="hit-note">{hit.note}</div>}
    </div>
  );
}

function BuildCard({
  status,
  onBuild,
  building,
}: {
  status: RagStatus | null;
  onBuild: () => void;
  building: boolean;
}) {
  if (!status) return null;

  if (building) {
    return (
      <div className="build-card building">
        <div className="build-title">
          正在构建知识库（{status.stage === "embedding" ? "向量化" : "收集词条"}）
          … {status.progress >= 0.1 ? `${Math.round(status.progress * 100)}%` : ""}
        </div>
        <div className="progress-track">
          <div
            className="progress-fill"
            style={{ width: `${Math.round(status.progress * 100)}%` }}
          />
        </div>
        {status.error && <div className="error">{status.error}</div>}
      </div>
    );
  }

  return (
    <div className="build-card">
      <div className="build-title">
        {status.built ? (
          <>
            知识库就绪：{status.dict_entries} 词条 +{" "}
            {status.scene_entries} 场景表达
            {status.embedding_model
              ? ` · ${status.embedding_model}`
              : " · 未配置向量模型（仅关键词检索）"}
          </>
        ) : (
          <>尚未构建知识库</>
        )}
      </div>
      <button className="primary-btn" onClick={onBuild}>
        {status.built ? "重建知识库" : "构建知识库"}
      </button>
      {status.error && <div className="error">{status.error}</div>}
    </div>
  );
}

export default function LookupPage({
  providerReady,
}: {
  providerReady: boolean;
}) {
  const [tab, setTab] = useState<"concept" | "scene">("concept");
  const [query, setQuery] = useState("");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [concept, setConcept] = useState<ConceptResult | null>(null);
  const [refining, setRefining] = useState(false);
  const [scene, setScene] = useState("business_email");
  const [sceneResult, setSceneResult] = useState<SceneResult | null>(null);
  const [status, setStatus] = useState<RagStatus | null>(null);
  const pollRef = useRef<number | null>(null);

  const refreshStatus = useCallback(async () => {
    try {
      setStatus(await fetchRagStatus());
    } catch {
      /* status card is best-effort */
    }
  }, []);

  useEffect(() => {
    void refreshStatus();
  }, [refreshStatus]);

  // poll while building
  useEffect(() => {
    if (status?.building) {
      pollRef.current = window.setInterval(() => void refreshStatus(), 800);
    } else if (pollRef.current) {
      window.clearInterval(pollRef.current);
      pollRef.current = null;
    }
    return () => {
      if (pollRef.current) window.clearInterval(pollRef.current);
    };
  }, [status?.building, refreshStatus]);

  async function onBuild() {
    try {
      await buildRag();
      await refreshStatus();
    } catch (err) {
      setError(String(err));
    }
  }

  async function onSearch() {
    const text = query.trim();
    if (!text || busy) return;

    setBusy(true);
    setError(null);

    try {
      if (tab === "concept") {
        setConcept(await lookupConcept(text));
      } else {
        setSceneResult(await lookupScene(scene, text));
      }
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(false);
    }
  }

  async function onRefine() {
    if (!concept || refining) return;

    setRefining(true);

    try {
      setConcept(await lookupConcept(concept.query, true));
    } catch (err) {
      setError(String(err));
    } finally {
      setRefining(false);
    }
  }

  async function onSceneBrowse(target: string) {
    setScene(target);
    setBusy(true);
    setError(null);

    try {
      setSceneResult(await lookupScene(target, ""));
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="lookup-page">
      <BuildCard
        status={status}
        onBuild={() => void onBuild()}
        building={!!status?.building}
      />

      <div className="tab-row">
        <button
          className={"tab-btn" + (tab === "concept" ? " active" : "")}
          onClick={() => setTab("concept")}
        >
          概念反查
        </button>
        <button
          className={"tab-btn" + (tab === "scene" ? " active" : "")}
          onClick={() => {
            setTab("scene");
            if (!sceneResult) void onSceneBrowse(scene);
          }}
        >
          场景表达
        </button>
      </div>

      {tab === "concept" ? (
        <>
          <div className="search-row">
            <input
              className="search-input"
              value={query}
              placeholder='用中文描述语义，如"形容说话尖酸刻薄但有道理"'
              onChange={(e) => setQuery(e.target.value)}
              onKeyDown={(e) => e.key === "Enter" && void onSearch()}
            />
            <button
              className="primary-btn"
              onClick={() => void onSearch()}
              disabled={busy}
            >
              {busy ? "检索中…" : "反查"}
            </button>
          </div>

          {error && <div className="error banner">{error}</div>}

          {concept && (
            <>
              {concept.results.length > 0 && (
                <>
                  <div className="result-toolbar">
                    <span>
                      「{concept.query}」的候选词（按融合排序）
                    </span>
                    <button
                      className="ghost-btn"
                      disabled={refining || !providerReady}
                      onClick={() => void onRefine()}
                      title={
                        providerReady
                          ? "让 LLM 从候选中精选并解释"
                          : "需要先配置 AI 供应商"
                      }
                    >
                      {refining ? "AI 精炼中…" : "AI 精炼"}
                    </button>
                  </div>
                  <div className="hit-list">
                    {concept.results.map((hit) => (
                      <HitCard key={hit.entry_id} hit={hit} />
                    ))}
                  </div>
                </>
              )}
              {concept.results.length === 0 && (
                <p className="placeholder">
                  没有命中。知识库未构建或未配置向量模型时，仅支持
                  3 个字符以上的关键词描述。
                </p>
              )}
              {concept.refinement && (
                <section className="memory-section">
                  <h3>AI 精炼</h3>
                  <div className="markdown">
                    <ReactMarkdown>{concept.refinement}</ReactMarkdown>
                  </div>
                </section>
              )}
              {concept.refinement_error && (
                <div className="error banner">
                  AI 精炼失败：{concept.refinement_error}
                </div>
              )}
            </>
          )}
        </>
      ) : (
        <>
          <div className="scene-tabs">
            {SCENES.map((s) => (
              <button
                key={s.key}
                className={
                  "scene-chip" + (scene === s.key ? " active" : "")}
                onClick={() => void onSceneBrowse(s.key)}
              >
                {s.label}
              </button>
            ))}
          </div>
          <div className="search-row">
            <input
              className="search-input"
              value={query}
              placeholder="搜索场景表达（中英文均可，留空浏览全部）"
              onChange={(e) => setQuery(e.target.value)}
              onKeyDown={(e) => e.key === "Enter" && void onSearch()}
            />
            <button
              className="primary-btn"
              onClick={() => void onSearch()}
              disabled={busy}
            >
              {busy ? "检索中…" : "搜索"}
            </button>
          </div>

          {error && <div className="error banner">{error}</div>}

          {sceneResult && (
            <div className="hit-list">
              {sceneResult.results.map((hit) => (
                <HitCard key={hit.entry_id} hit={hit} />
              ))}
            </div>
          )}
        </>
      )}
    </div>
  );
}
