"""Corpus builder: dictionary subset + scenario bank -> corpus.db.

Layout (corpus.db under the shared sidecar data dir; word knowledge is
user-independent, so one corpus serves all users):

    entries       metadata rows (dict words + scene expressions)
    entries_fts   FTS5 trigram index over "word translation note"
    entries_vec   sqlite-vec table (dimension fixed by embedding model)
    embed_cache   (model, content-hash) -> vector, so rebuilds and
                  interrupted builds reuse paid embeddings
    kv            build metadata / progress

Connection model: entries/FTS/cache live on the plain connection; the
vec virtual table is only ever touched through a dedicated connection
with the sqlite-vec extension loaded (its schema persists in the file).

Degradation: with no embedding model configured the corpus is built
BM25-only (scene search stays fully usable, concept lookup loses the
semantic channel). With TRANCE_AI_EMBED_MOCK=1 deterministic hash
vectors are used - pipeline testing without API cost.
"""

from __future__ import annotations

import hashlib
import math
import os
import sqlite3
import struct
import threading
import time
from dataclasses import dataclass
from pathlib import Path

from ..session import get_dict_db_path
from .scenes_seed import ENTRIES as SCENE_ENTRIES

DEFAULT_TOP_N = 30000
EMBED_BATCH = 128


@dataclass
class BuildState:
    running: bool = False
    error: str = ""
    progress: float = 0.0  # 0..1
    stage: str = ""  # collecting | embedding | done
    total: int = 0
    done: int = 0


build_state = BuildState()


# ---------------------------------------------------------------- helpers


def content_hash(text: str) -> str:
    return hashlib.sha1(text.encode("utf-8")).hexdigest()


def mock_embedding(text: str, dim: int = 256) -> list[float]:
    """Deterministic bag-of-token hash embedding (pipeline testing only)."""
    vec = [0.0] * dim

    for token in text.lower().split():
        h = int(hashlib.md5(token.encode()).hexdigest(), 16)
        vec[h % dim] += 1.0

    norm = math.sqrt(sum(v * v for v in vec)) or 1.0
    return [v / norm for v in vec]


def serialize_f32(vec: list[float]) -> bytes:
    return struct.pack(f"{len(vec)}f", *vec)


def _open_vec_connection(db_path: Path) -> sqlite3.Connection:
    conn = sqlite3.connect(db_path, timeout=10, check_same_thread=False)
    conn.enable_load_extension(True)
    import sqlite_vec

    sqlite_vec.load(conn)
    return conn


# ---------------------------------------------------------------- schema

DICT_QUERY = """
SELECT word, part_of_speech, frequency, translation FROM (
    SELECT w.word AS word, w.part_of_speech AS part_of_speech,
           w.frequency AS frequency,
           (SELECT t.target_word FROM word_translations t
             WHERE t.source_word = w.word
               AND t.source_language = 'en'
               AND t.target_language = 'zh'
             LIMIT 1) AS translation
    FROM words w
) WHERE translation IS NOT NULL AND translation != ''
ORDER BY frequency DESC, word ASC
LIMIT ?
"""

SCHEMA = """
CREATE TABLE IF NOT EXISTS entries(
    entry_id INTEGER PRIMARY KEY,
    kind TEXT NOT NULL,          -- 'dict' | 'scene'
    word TEXT NOT NULL,          -- dict: word; scene: EN expression
    translation TEXT NOT NULL,   -- dict: zh gloss; scene: zh meaning
    note TEXT DEFAULT '',        -- dict: pos; scene: usage note
    frequency INTEGER DEFAULT 0,
    scene TEXT DEFAULT ''
);
CREATE TABLE IF NOT EXISTS embed_cache(
    cache_key TEXT PRIMARY KEY,  -- model + ':' + content hash
    dim INTEGER NOT NULL,
    embedding BLOB NOT NULL
);
CREATE TABLE IF NOT EXISTS kv(key TEXT PRIMARY KEY, value TEXT NOT NULL);
CREATE INDEX IF NOT EXISTS idx_entries_kind ON entries(kind);
"""

# plain (self-contained) FTS5 with the trigram tokenizer: substring
# matching that also works for CJK (unicode61 would treat a whole CJK
# run as one token and never match partial queries)
FTS_DDL = (
    "CREATE VIRTUAL TABLE entries_fts USING fts5("
    "text, tokenize='trigram')"
)


class CorpusStore:
    """Owns corpus.db; one instance per sidecar process.

    Connections use check_same_thread=False plus this lock: the build
    runs on the event-loop thread while sync endpoints hit the store
    from FastAPI's thread pool, so every access must be serialized.
    """

    def __init__(self, db_path: Path) -> None:
        self.db_path = db_path
        self.lock = threading.RLock()
        db_path.parent.mkdir(parents=True, exist_ok=True)
        self.db = sqlite3.connect(db_path, timeout=10,
                                  check_same_thread=False)
        self.db.row_factory = sqlite3.Row
        # WAL: build writes and concurrent status/search reads coexist
        self.db.execute("PRAGMA journal_mode=WAL")
        self.db.execute("PRAGMA busy_timeout=10000")
        with self.lock:
            self.db.executescript(SCHEMA)
            self._ensure_fts()
        # lazily opened by build_corpus or the searcher when a vec table
        # already exists in the file
        self.vec_conn: sqlite3.Connection | None = None

    def close(self) -> None:
        with self.lock:
            if self.vec_conn is not None:
                self.vec_conn.close()
            self.db.close()

    # ---------- schema ----------

    def _ensure_fts(self) -> None:
        row = self.db.execute(
            "SELECT sql FROM sqlite_master WHERE name='entries_fts'"
        ).fetchone()

        if row is None:
            self.db.execute(FTS_DDL)
        elif "trigram" not in (row["sql"] or ""):
            # pre-trigram table (older build): rebuild it
            self.db.execute("DROP TABLE entries_fts")
            self.db.execute(FTS_DDL)

    # ---------- kv ----------

    def kv_get(self, key: str, default: str = "") -> str:
        with self.lock:
            row = self.db.execute(
                "SELECT value FROM kv WHERE key=?", (key,)
            ).fetchone()
            return row["value"] if row else default

    def kv_set(self, key: str, value: str) -> None:
        with self.lock:
            with self.db:
                self.db.execute(
                    "INSERT OR REPLACE INTO kv(key, value) VALUES (?, ?)",
                    (key, value),
                )

    # ---------- text / stats ----------

    @staticmethod
    def entry_text(row: sqlite3.Row) -> str:
        parts = [row["word"], row["translation"]]

        if row["note"]:
            parts.append(row["note"])

        if row["scene"]:
            parts.append(row["scene"])

        return " ".join(parts)

    def stats(self) -> dict:
        with self.lock:
            total = self.db.execute(
                "SELECT COUNT(*) FROM entries"
            ).fetchone()[0]
            dict_n = self.db.execute(
                "SELECT COUNT(*) FROM entries WHERE kind='dict'"
            ).fetchone()[0]
            scene_n = self.db.execute(
                "SELECT COUNT(*) FROM entries WHERE kind='scene'"
            ).fetchone()[0]
            embedded = self.db.execute(
                "SELECT COUNT(*) FROM embed_cache"
            ).fetchone()[0]

        return {
            "built": total > 0,
            "total": total,
            "dict_entries": dict_n,
            "scene_entries": scene_n,
            "embedded": embedded,
            "embedding_model": self.kv_get("embedding_model"),
            "dim": int(self.kv_get("dim", "0") or 0),
        }

    def get_vec_conn(self) -> sqlite3.Connection | None:
        """Open (once) the vec-enabled connection; None when the corpus
        has no vec table yet."""
        with self.lock:
            if self.vec_conn is not None:
                return self.vec_conn

            has_vec = self.db.execute(
                "SELECT name FROM sqlite_master WHERE type='table' "
                "AND name='entries_vec'"
            ).fetchone()

            if not has_vec:
                return None

            self.vec_conn = _open_vec_connection(self.db_path)
            return self.vec_conn


# ---------------------------------------------------------------- builder


async def build_corpus(
    store: CorpusStore, top_n: int = DEFAULT_TOP_N
) -> dict:
    """(Re)build entries/FTS/vec. Entry collection is a full local
    refresh; embeddings go through the content cache so nothing is paid
    twice across rebuilds."""
    from ..llm import NoProviderError, client_for, current_provider

    build_state.running = True
    build_state.error = ""
    build_state.progress = 0.0
    build_state.stage = "collecting"
    build_state.done = 0
    build_state.total = 0

    try:
        # ---- 1. collect entries ------------------------------------
        dict_path = get_dict_db_path()
        rows: list[tuple] = []

        if dict_path and Path(dict_path).exists():
            # Read-only: the host owns this database.
            ro = sqlite3.connect(
                f"file:{Path(dict_path).as_posix()}?mode=ro", uri=True
            )
            ro.row_factory = sqlite3.Row

            for r in ro.execute(DICT_QUERY, (top_n,)):
                rows.append((
                    "dict", r["word"], r["translation"],
                    r["part_of_speech"] or "", int(r["frequency"] or 0), "",
                ))
            ro.close()

        for scene, en, zh, note in SCENE_ENTRIES:
            rows.append(("scene", en, zh, note, 0, scene))

        with store.lock, store.db:
            store.db.execute("DELETE FROM entries")
            store.db.executemany(
                "INSERT INTO entries(kind, word, translation, note, "
                "frequency, scene) VALUES (?, ?, ?, ?, ?, ?)",
                rows,
            )

        # ---- 2. FTS (trigram) --------------------------------------
        with store.lock, store.db:
            store.db.execute("DELETE FROM entries_fts")

            for row in store.db.execute(
                "SELECT entry_id, word, translation, note, scene FROM entries"
            ).fetchall():
                store.db.execute(
                    "INSERT INTO entries_fts(rowid, text) VALUES (?, ?)",
                    (row["entry_id"], store.entry_text(row)),
                )

        build_state.total = len(rows)
        build_state.progress = 0.1
        build_state.stage = "embedding"

        # ---- 3. embeddings -----------------------------------------
        use_mock = os.environ.get("TRANCE_AI_EMBED_MOCK") == "1"

        try:
            provider = current_provider()
            has_embed = bool(provider.embedding_model)
        except NoProviderError:
            provider = None
            has_embed = False

        model_name = (
            provider.embedding_model if provider and has_embed else "mock"
        )

        with store.lock:
            entries = store.db.execute(
                "SELECT entry_id, word, translation, note, scene FROM entries"
            ).fetchall()

        done = 0
        dim = 0
        vec_conn: sqlite3.Connection | None = None

        for batch_start in range(0, len(entries), EMBED_BATCH):
            batch = entries[batch_start:batch_start + EMBED_BATCH]
            texts = [store.entry_text(r) for r in batch]
            vectors: list[list[float]]

            if use_mock or not has_embed:
                vectors = [mock_embedding(t) for t in texts]
            else:
                client = client_for(provider)
                response = await client.embeddings.create(
                    model=provider.embedding_model, input=texts
                )
                vectors = [item.embedding for item in response.data]

            dim = len(vectors[0]) if vectors else dim

            with store.lock:
                if vec_conn is None and dim:
                    vec_conn = _open_vec_connection(store.db_path)

                    # vec table is rebuilt per build (dim may change)
                    vec_conn.execute("DROP TABLE IF EXISTS entries_vec")
                    vec_conn.execute(
                        f"CREATE VIRTUAL TABLE entries_vec "
                        f"USING vec0(embedding float[{dim}])"
                    )
                    vec_conn.commit()

                # cache writes first (own transaction) ...
                with store.db:
                    for row, text, vec in zip(batch, texts, vectors):
                        key = f"{model_name}:{content_hash(text)}"
                        store.db.execute(
                            "INSERT OR REPLACE INTO embed_cache"
                            "(cache_key, dim, embedding) VALUES (?, ?, ?)",
                            (key, len(vec), serialize_f32(vec)),
                        )

                # ... then vec writes: two connections must never hold
                # write transactions on the same file at once
                if vec_conn is not None:
                    with vec_conn:
                        for row, vec in zip(batch, vectors):
                            vec_conn.execute(
                                "INSERT OR REPLACE INTO entries_vec"
                                "(rowid, embedding) VALUES (?, ?)",
                                (row["entry_id"], serialize_f32(vec)),
                            )

            done += len(batch)
            build_state.done = done
            build_state.progress = 0.1 + 0.9 * done / max(len(entries), 1)

        with store.lock:
            if store.vec_conn is not None:
                store.vec_conn.close()

            store.vec_conn = vec_conn

            store.kv_set("embedding_model", model_name)
            store.kv_set("dim", str(dim))
            store.kv_set("total_entries", str(len(entries)))
            store.kv_set("built_at", time.strftime("%Y-%m-%dT%H:%M:%S"))

        build_state.stage = "done"
        build_state.progress = 1.0
        return store.stats()
    except Exception as exc:
        build_state.error = f"{type(exc).__name__}: {exc}"
        raise
    finally:
        build_state.running = False
