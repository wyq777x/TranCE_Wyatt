"""Hybrid retrieval over the native RAG corpus.

Channel 1  FTS5 trigram (BM25)  substring/keyword matching, covers CJK
Channel 2  sqlite-vec KNN       semantic matching via embeddings
Fusion     Reciprocal Rank Fusion (RRF, k=60)

Query embedding reuses the corpus embed cache when possible; with no
embedding channel available the search degrades to BM25-only.
"""

from __future__ import annotations

import sqlite3
from dataclasses import dataclass, field

import openai

from ..llm import current_provider
from .corpus import CorpusStore, content_hash, mock_embedding, serialize_f32

RRF_K = 60


@dataclass
class Hit:
    entry_id: int
    kind: str
    word: str
    translation: str
    note: str
    frequency: int
    scene: str
    score: float = 0.0
    sources: list[str] = field(default_factory=list)
    vec_distance: float | None = None


def _fts_quote(query: str) -> str:
    """Escape user input into a single FTS5 phrase."""
    return '"' + query.replace('"', '""') + '"'


class HybridSearcher:
    def __init__(self, store: CorpusStore) -> None:
        self.store = store

    # ---------- channels ----------

    def _bm25(self, query: str, limit: int) -> list[int]:
        # trigram needs >= 3 characters to match anything
        if len(query.strip()) < 3:
            return []

        try:
            with self.store.lock:
                rows = self.store.db.execute(
                    "SELECT rowid FROM entries_fts "
                    "WHERE entries_fts MATCH ? "
                    "ORDER BY bm25(entries_fts) LIMIT ?",
                    (_fts_quote(query.strip()), limit),
                ).fetchall()
            return [r["rowid"] for r in rows]
        except sqlite3.OperationalError:
            # malformed query safety net
            return []

    def _query_vector(self, query: str) -> list[float] | None:
        model = self.store.kv_get("embedding_model")

        if not model:
            return None

        # cache hit: identical query text costs nothing
        key = f"{model}:{content_hash('QUERY::' + query)}"

        with self.store.lock:
            row = self.store.db.execute(
                "SELECT embedding FROM embed_cache WHERE cache_key=?", (key,)
            ).fetchone()

        if row:
            import struct

            blob = row["embedding"]
            dim = len(blob) // 4
            return list(struct.unpack(f"{dim}f", blob))

        try:
            provider = current_provider()

            if not provider.embedding_model:
                return None

            # synchronous client: this runs in worker threads (sync
            # endpoints), where the async client has no event loop
            client = openai.OpenAI(
                base_url=provider.base_url, api_key=provider.api_key
            )
            response = client.embeddings.create(
                model=provider.embedding_model, input=[query]
            )
            vec = list(response.data[0].embedding)
        except Exception:
            # semantic channel unavailable (no provider / offline) ->
            # fall back to a mock vector only when corpus is mock-built
            if model == "mock":
                vec = mock_embedding("QUERY::" + query)
            else:
                return None

        with self.store.lock:
            self.store.db.execute(
                "INSERT OR REPLACE INTO embed_cache"
                "(cache_key, dim, embedding) VALUES (?, ?, ?)",
                (key, len(vec), serialize_f32(vec)),
            )
            self.store.db.commit()

        return vec

    def _knn(self, query: str, limit: int) -> list[tuple[int, float]]:
        vec_conn = self.store.get_vec_conn()

        if vec_conn is None:
            return []

        vec = self._query_vector(query)

        if vec is None:
            return []

        try:
            with self.store.lock:
                rows = vec_conn.execute(
                    "SELECT rowid, distance FROM entries_vec "
                    "WHERE embedding MATCH ? AND k = ?",
                    (serialize_f32(vec), limit),
                ).fetchall()
            return [(r[0], r[1]) for r in rows]
        except sqlite3.OperationalError:
            return []

    # ---------- fusion ----------

    def search(
        self,
        query: str,
        top_k: int = 10,
        kind: str | None = None,
        scene: str | None = None,
    ) -> list[Hit]:
        candidate_limit = max(top_k * 4, 40)

        bm25_ids = self._bm25(query, candidate_limit)
        knn_hits = self._knn(query, candidate_limit)

        # RRF
        scores: dict[int, float] = {}
        sources: dict[int, list[str]] = {}
        distances: dict[int, float] = {}

        for rank, entry_id in enumerate(bm25_ids):
            scores[entry_id] = scores.get(entry_id, 0.0) + 1.0 / (
                RRF_K + rank + 1
            )
            sources.setdefault(entry_id, []).append("bm25")

        for rank, (entry_id, distance) in enumerate(knn_hits):
            scores[entry_id] = scores.get(entry_id, 0.0) + 1.0 / (
                RRF_K + rank + 1
            )
            sources.setdefault(entry_id, []).append("vec")
            distances[entry_id] = distance

        # no-channel case (e.g. very short query without embeddings)
        if not scores and scene and not query.strip():
            return self._scene_entries(scene, top_k)

        if not scores:
            return []

        ranked = sorted(scores.items(), key=lambda kv: -kv[1])[: candidate_limit]

        ids = [entry_id for entry_id, _ in ranked]
        placeholders = ",".join("?" * len(ids))

        with self.store.lock:
            rows = {
                r["entry_id"]: r
                for r in self.store.db.execute(
                    f"SELECT * FROM entries WHERE entry_id IN ({placeholders})",
                    ids,
                ).fetchall()
            }

        hits: list[Hit] = []

        for entry_id, score in ranked:
            row = rows.get(entry_id)

            if row is None:
                continue

            if kind and row["kind"] != kind:
                continue

            if scene and row["scene"] != scene:
                continue

            hits.append(
                Hit(
                    entry_id=entry_id,
                    kind=row["kind"],
                    word=row["word"],
                    translation=row["translation"],
                    note=row["note"],
                    frequency=row["frequency"],
                    scene=row["scene"],
                    score=round(score, 6),
                    sources=sources.get(entry_id, []),
                    vec_distance=(
                        round(distances[entry_id], 4)
                        if entry_id in distances
                        else None
                    ),
                )
            )

            if len(hits) >= top_k:
                break

        return hits

    def _scene_entries(self, scene: str, limit: int) -> list[Hit]:
        with self.store.lock:
            rows = self.store.db.execute(
                "SELECT * FROM entries WHERE kind='scene' AND scene=? "
                "ORDER BY entry_id LIMIT ?",
                (scene, limit),
            ).fetchall()
        return [
            Hit(
                entry_id=r["entry_id"], kind=r["kind"], word=r["word"],
                translation=r["translation"], note=r["note"],
                frequency=r["frequency"], scene=r["scene"],
                sources=["list"],
            )
            for r in rows
        ]
