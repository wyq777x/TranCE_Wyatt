import { useCallback, useEffect, useRef, useState } from "react";
import * as echarts from "echarts";
import ReactMarkdown from "react-markdown";
import {
  expandMesh,
  explainMesh,
  type MeshExpandResult,
  type MeshNode,
} from "../api";

const TYPE_COLORS: Record<string, string> = {
  center: "#00E5FF",
  prefix: "#FFB84F",
  suffix: "#FF9F6E",
  root: "#B388FF",
  synonym: "#4FD387",
  antonym: "#FF6B6B",
  related: "#7FB2FF",
  family: "#8FA3BD",
};

const TYPE_LABELS: Record<string, string> = {
  center: "中心词",
  prefix: "前缀",
  suffix: "后缀",
  root: "词根",
  synonym: "同义",
  antonym: "反义",
  related: "联想",
  family: "词族",
};

const RELATION_LABELS: Record<string, string> = {
  has_prefix: "前缀",
  has_root: "词根",
  has_suffix: "后缀",
  synonym: "同义",
  antonym: "反义",
  related: "联想",
  family: "词族",
};

export default function MeshPage({
  providerReady,
}: {
  providerReady: boolean;
}) {
  const [word, setWord] = useState("");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [result, setResult] = useState<MeshExpandResult | null>(null);
  const [selected, setSelected] = useState<MeshNode | null>(null);
  const [explaining, setExplaining] = useState(false);
  const [explanation, setExplanation] = useState("");
  const chartRef = useRef<HTMLDivElement>(null);
  const chart = useRef<echarts.ECharts | null>(null);
  const expandWordRef = useRef<(w: string) => void>(() => {});

  useEffect(() => {
    if (chartRef.current && !chart.current) {
      chart.current = echarts.init(chartRef.current);
      chart.current.on("click", (params) => {
        if (params.dataType === "node") {
          const node = (params.data as { nodeRef?: MeshNode }).nodeRef;

          if (node) {
            setSelected(node);
          }
        }
      });
    }

    const onResize = () => chart.current?.resize();
    window.addEventListener("resize", onResize);
    return () => window.removeEventListener("resize", onResize);
  }, []);

  const renderGraph = useCallback((data: MeshExpandResult) => {
    if (!chart.current) return;

    const echartsNodes = data.nodes.map((n) => ({
      id: n.id,
      name: n.label,
      symbolSize:
        n.type === "center" ? 52 : n.type === "family" ? 24 : 34,
      category: n.type,
      itemStyle: {
        color: TYPE_COLORS[n.type] ?? "#8FA3BD",
        shadowBlur: n.type === "center" ? 18 : 0,
        shadowColor: TYPE_COLORS[n.type] ?? "#8FA3BD",
      },
      label: { show: true, color: "#dbe4f0", fontSize: 12 },
      nodeRef: n,
    }));

    const echartsEdges = data.edges.map((e) => ({
      source: e.source,
      target: e.target,
      value: RELATION_LABELS[e.relation] ?? e.relation,
      lineStyle: {
        color: "rgba(143, 163, 189, 0.35)",
        width: e.relation === "family" ? 1 : 1.6,
        curveness: 0.12,
      },
    }));

    chart.current.setOption(
      {
        backgroundColor: "transparent",
        tooltip: {
          backgroundColor: "#172238",
          borderColor: "rgba(0, 229, 255, 0.3)",
          textStyle: { color: "#dbe4f0", fontSize: 12 },
          formatter: (p: {
            dataType: string;
            data: { nodeRef?: MeshNode; value?: string };
          }) => {
            if (p.dataType === "edge") {
              return p.data.value ?? "";
            }
            const n = p.data.nodeRef;
            return n
              ? `<b>${n.label}</b><br/>${TYPE_LABELS[n.type] ?? ""}${
                  n.meaning ? "<br/>" + n.meaning : ""
                }`
              : "";
          },
        },
        series: [
          {
            type: "graph",
            layout: "force",
            roam: true,
            draggable: true,
            data: echartsNodes,
            links: echartsEdges,
            force: {
              repulsion: 320,
              edgeLength: [60, 130],
              gravity: 0.08,
            },
            edgeLabel: { show: false },
            emphasis: { focus: "adjacency" },
          },
        ],
      },
      { notMerge: true },
    );
  }, []);

  const doExpand = useCallback(
    async (target: string) => {
      const w = target.trim().toLowerCase();
      if (!w || busy) return;

      setBusy(true);
      setError(null);
      setSelected(null);
      setExplanation("");

      try {
        const data = await expandMesh(w);
        setResult(data);
        renderGraph(data);
      } catch (err) {
        setError(String(err));
      } finally {
        setBusy(false);
      }
    },
    [busy, renderGraph],
  );

  useEffect(() => {
    expandWordRef.current = (w: string) => void doExpand(w);
  }, [doExpand]);

  async function onExplain() {
    if (!result || explaining) return;

    setExplaining(true);
    setError(null);

    try {
      setExplanation(await explainMesh(result.word, result.morphemes));
    } catch (err) {
      setError(String(err));
    } finally {
      setExplaining(false);
    }
  }

  const expandable = (node: MeshNode) =>
    node.type !== "center" && node.type !== "family";

  return (
    <div className="mesh-page">
      <div className="mesh-toolbar">
        <input
          className="search-input"
          value={word}
          placeholder="输入一个英文词，发散它的词根/词缀/近反义网络"
          onChange={(e) => setWord(e.target.value)}
          onKeyDown={(e) => e.key === "Enter" && void doExpand(word)}
        />
        <button
          className="primary-btn"
          onClick={() => void doExpand(word)}
          disabled={busy}
        >
          {busy ? "展开中…" : "发散"}
        </button>
        {result && (
          <button
            className="ghost-btn"
            disabled={explaining || !providerReady}
            onClick={() => void onExplain()}
            title={
              providerReady ? "词源故事 + 记忆路径讲解" : "需要先配置 AI 供应商"
            }
          >
            {explaining ? "AI 讲解中…" : "AI 讲解"}
          </button>
        )}
      </div>

      <div className="mesh-legend">
        {Object.entries(TYPE_LABELS)
          .filter(([t]) => t !== "center")
          .map(([t, label]) => (
            <span key={t} className="legend-item">
              <span
                className="legend-dot"
                style={{ background: TYPE_COLORS[t] }}
              />
              {label}
            </span>
          ))}
      </div>

      {error && <div className="error banner">{error}</div>}
      {result?.llm_error && !result.llm_used && (
        <div className="warn banner">
          近反义层不可用（{result.llm_error}）——当前展示离线词根词缀网络。
        </div>
      )}

      <div className="mesh-body">
        <div ref={chartRef} className="mesh-chart" />
        <aside className="mesh-side">
          {selected ? (
            <div className="node-detail">
              <h3 style={{ color: TYPE_COLORS[selected.type] }}>
                {selected.label}
              </h3>
              <div className="node-type">{TYPE_LABELS[selected.type]}</div>
              {selected.meaning && (
                <p className="node-meaning">{selected.meaning}</p>
              )}
              {selected.detail && (
                <p className="node-detail-text">{selected.detail}</p>
              )}
              {expandable(selected) && (
                <button
                  className="primary-btn"
                  onClick={() => {
                    setWord(selected.label);
                    expandWordRef.current(selected.label);
                  }}
                >
                  以此词为中心发散
                </button>
              )}
            </div>
          ) : (
            <div className="node-detail placeholder">
              点击图中的节点查看详情；
              <br />
              词素/同义词节点可以继续发散。
            </div>
          )}

          {result && result.morphemes.length > 0 && (
            <div className="morpheme-list">
              <h4>词素分解</h4>
              {result.morphemes.map((m, i) => (
                <div key={i} className="morpheme-row">
                  <span className={"morpheme-tag tag-" + m.kind}>
                    {m.kind === "prefix" ? "前缀" : m.kind === "suffix" ? "后缀" : "词根"}{" "}
                    {m.text}
                  </span>
                  <span className="dim">{m.meaning}</span>
                </div>
              ))}
            </div>
          )}
        </aside>
      </div>

      {explanation && (
        <section className="memory-section">
          <h3>AI 讲解 · {result?.word}</h3>
          <div className="markdown">
            <ReactMarkdown>{explanation}</ReactMarkdown>
          </div>
        </section>
      )}
    </div>
  );
}
