"""Per-user learner mastery model.

Backed by SQLite (learner.db under the user's sidecar data directory).
Two signal kinds feed the model:

  - snapshot   initial/periodic state pushed by the Qt host on startup:
               vocabulary status, favorites, recite/search history
  - events     incremental in-session behaviour: quiz answers, lookups,
               recite, favorites, explicit status changes

Mastery values are heuristic [0, 1] scores; wrong answers dominate
(they mark a word as a weakness) while lookups/recite nudge slightly.
"""

from __future__ import annotations

import json
import sqlite3
import threading
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Optional

SCHEMA = """
CREATE TABLE IF NOT EXISTS word_mastery(
    word TEXT PRIMARY KEY,
    mastery REAL NOT NULL DEFAULT 0.0,
    srs_stage INTEGER NOT NULL DEFAULT 0,
    correct_count INTEGER NOT NULL DEFAULT 0,
    wrong_count INTEGER NOT NULL DEFAULT 0,
    lookups INTEGER NOT NULL DEFAULT 0,
    recites INTEGER NOT NULL DEFAULT 0,
    favorite INTEGER NOT NULL DEFAULT 0,
    sources TEXT NOT NULL DEFAULT '',
    first_seen TEXT NOT NULL,
    last_seen TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS events(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ts TEXT NOT NULL,
    type TEXT NOT NULL,
    word TEXT NOT NULL,
    correct INTEGER,
    meta TEXT NOT NULL DEFAULT ''
);
CREATE TABLE IF NOT EXISTS kv(
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_mastery_weak
    ON word_mastery(mastery ASC, wrong_count DESC);
CREATE TABLE IF NOT EXISTS quiz_history(
    quiz_id INTEGER PRIMARY KEY AUTOINCREMENT,
    created_at TEXT NOT NULL,
    mode TEXT NOT NULL,             -- cloze | story
    words_json TEXT NOT NULL,       -- target words of this quiz
    quiz_json TEXT NOT NULL,       -- full generated quiz object
    submitted INTEGER NOT NULL DEFAULT 0,
    score_json TEXT DEFAULT ''
);
"""

# mastery thresholds
WEAK_MASTERY = 0.45
WEAK_MIN_WRONG = 2

def _locked(fn):
    """Serialize DB access across threads (see class docstring)."""
    import functools

    @functools.wraps(fn)
    def wrapper(self, *args, **kwargs):
        with self.lock:
            return fn(self, *args, **kwargs)

    return wrapper


DEFAULT_SNAPSHOT_LEARNING = 0.25
DEFAULT_SNAPSHOT_MASTERED = 0.9


def _now() -> str:
    return time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime())


@dataclass
class WordStats:
    word: str
    mastery: float
    srs_stage: int
    correct_count: int
    wrong_count: int
    lookups: int
    recites: int
    favorite: bool


class LearnerStore:
    """Per-user mastery store.

    Connections use check_same_thread=False plus this lock: sync
    endpoints (thread pool) and async endpoints (event loop) share one
    store, so access must be serialized.
    """

    def __init__(self, db_path: Path) -> None:
        db_path.parent.mkdir(parents=True, exist_ok=True)
        self.lock = threading.RLock()
        self.db = sqlite3.connect(db_path, timeout=10,
                                  check_same_thread=False)
        self.db.row_factory = sqlite3.Row
        self.db.execute("PRAGMA journal_mode=WAL")
        self.db.execute("PRAGMA busy_timeout=10000")

        with self.lock:
            self.db.executescript(SCHEMA)
            self.db.commit()

    # ---------- low-level helpers ----------

    @_locked

    def _touch(self, word: str, source: str) -> sqlite3.Row:
        row = self.db.execute(
            "SELECT * FROM word_mastery WHERE word = ?", (word,)
        ).fetchone()

        if row is None:
            now = _now()
            self.db.execute(
                "INSERT INTO word_mastery(word, first_seen, last_seen, "
                "sources) VALUES (?, ?, ?, ?)",
                (word, now, now, source),
            )
            row = self.db.execute(
                "SELECT * FROM word_mastery WHERE word = ?", (word,)
            ).fetchone()

        return row

    @_locked

    def _update(
        self,
        word: str,
        source: str,
        *,
        mastery: Optional[float] = None,
        srs_stage: Optional[int] = None,
        d_correct: int = 0,
        d_wrong: int = 0,
        d_lookups: int = 0,
        d_recites: int = 0,
        favorite: Optional[int] = None,
    ) -> None:
        row = self._touch(word, source)

        if source and source not in row["sources"].split(","):
            sources = (row["sources"] + "," + source).strip(",")
        else:
            sources = row["sources"]

        self.db.execute(
            "UPDATE word_mastery SET mastery=?, srs_stage=?, "
            "correct_count=?, wrong_count=?, lookups=?, recites=?, "
            "favorite=?, sources=?, last_seen=? WHERE word=?",
            (
                row["mastery"] if mastery is None else mastery,
                row["srs_stage"] if srs_stage is None else srs_stage,
                row["correct_count"] + d_correct,
                row["wrong_count"] + d_wrong,
                row["lookups"] + d_lookups,
                row["recites"] + d_recites,
                row["favorite"] if favorite is None else favorite,
                sources,
                _now(),
                word,
            ),
        )

    # ---------- snapshot ----------

    @_locked

    def apply_snapshot(self, snapshot: dict) -> None:
        """Reset the mastery table from the host's vocabulary state.

        History-based counters are preserved for words already known to
        the model so repeated snapshots don't inflate/erase evidence.
        """
        with self.db:
            for word in snapshot.get("vocabulary_mastered", []):
                row = self.db.execute(
                    "SELECT mastery FROM word_mastery WHERE word=?", (word,)
                ).fetchone()

                mastery = (
                    max(row["mastery"], DEFAULT_SNAPSHOT_MASTERED)
                    if row
                    else DEFAULT_SNAPSHOT_MASTERED
                )
                self._update(word, "vocabulary", mastery=mastery)

            for word in snapshot.get("vocabulary_learning", []):
                row = self.db.execute(
                    "SELECT mastery FROM word_mastery WHERE word=?", (word,)
                ).fetchone()

                mastery = (
                    min(row["mastery"], DEFAULT_SNAPSHOT_LEARNING)
                    if row
                    else DEFAULT_SNAPSHOT_LEARNING
                )
                self._update(word, "vocabulary", mastery=mastery)

            for word in snapshot.get("favorites", []):
                self._update(word, "favorite", favorite=1)

            for word in snapshot.get("recite_history", []):
                self._update(word, "recite", d_recites=1)

            for word in snapshot.get("search_history", []):
                self._update(word, "lookup", d_lookups=1)

    # ---------- events ----------

    @_locked

    def apply_event(self, event_type: str, word: str, **fields: object) -> None:
        correct = fields.get("correct")

        with self.db:
            self.db.execute(
                "INSERT INTO events(ts, type, word, correct) "
                "VALUES (?, ?, ?, ?)",
                (
                    _now(),
                    event_type,
                    word,
                    None if correct is None else int(bool(correct)),
                ),
            )

            if event_type == "quiz_answer":
                self._apply_quiz(word, bool(correct))
            elif event_type == "lookup":
                self._apply_seen(word, "lookup", d_lookups=1, gain=0.02)
            elif event_type == "recite":
                self._apply_seen(word, "recite", d_recites=1, gain=0.03)
            elif event_type == "favorite":
                self._update(word, "favorite",
                             favorite=int(bool(fields.get("favorite"))))
            elif event_type == "word_status":
                mastered = int(fields.get("status", 0)) == 1
                self._update(
                    word,
                    "vocabulary",
                    mastery=0.9 if mastered else 0.25,
                    srs_stage=2 if mastered else 0,
                )

    @_locked

    def _apply_quiz(self, word: str, correct: bool) -> None:
        row = self._touch(word, "quiz")

        if correct:
            mastery = row["mastery"] + (1.0 - row["mastery"]) * 0.25
            srs_stage = min(row["srs_stage"] + 1, 8)
            self._update(
                word,
                "quiz",
                mastery=mastery,
                srs_stage=srs_stage,
                d_correct=1,
            )
        else:
            mastery = row["mastery"] * 0.55
            self._update(
                word,
                "quiz",
                mastery=mastery,
                srs_stage=0,
                d_wrong=1,
            )

    @_locked

    def _apply_seen(
        self, word: str, source: str, *, d_lookups: int = 0,
        d_recites: int = 0, gain: float = 0.02,
    ) -> None:
        row = self._touch(word, source)
        mastery = min(row["mastery"] + gain, 0.6)
        self._update(
            word,
            source,
            mastery=mastery,
            d_lookups=d_lookups,
            d_recites=d_recites,
        )

    # ---------- queries ----------

    @_locked

    def weak_words(self, limit: int = 30) -> list[WordStats]:
        rows = self.db.execute(
            "SELECT * FROM word_mastery "
            "WHERE mastery < ? OR wrong_count >= ? "
            "ORDER BY wrong_count DESC, mastery ASC LIMIT ?",
            (WEAK_MASTERY, WEAK_MIN_WRONG, limit),
        ).fetchall()
        return [self._to_stats(r) for r in rows]

    @_locked

    def all_words(self, limit: int = 500) -> list[WordStats]:
        rows = self.db.execute(
            "SELECT * FROM word_mastery ORDER BY last_seen DESC LIMIT ?",
            (limit,),
        ).fetchall()
        return [self._to_stats(r) for r in rows]

    @_locked

    def stats(self) -> dict:
        row = self.db.execute(
            "SELECT COUNT(*) AS total, "
            "SUM(CASE WHEN mastery >= 0.8 THEN 1 ELSE 0 END) AS strong, "
            "SUM(CASE WHEN mastery < ? OR wrong_count >= ? "
            "         THEN 1 ELSE 0 END) AS weak "
            "FROM word_mastery",
            (WEAK_MASTERY, WEAK_MIN_WRONG),
        ).fetchone()

        events = self.db.execute("SELECT COUNT(*) FROM events").fetchone()[0]

        return {
            "total_words": row["total"] or 0,
            "strong_words": row["strong"] or 0,
            "weak_words": row["weak"] or 0,
            "events": events,
        }

    def _to_stats(self, row: sqlite3.Row) -> WordStats:
        return WordStats(
            word=row["word"],
            mastery=round(row["mastery"], 3),
            srs_stage=row["srs_stage"],
            correct_count=row["correct_count"],
            wrong_count=row["wrong_count"],
            lookups=row["lookups"],
            recites=row["recites"],
            favorite=bool(row["favorite"]),
        )

    # ---------- narrative (LLM memory profile) ----------

    @_locked

    def get_narrative(self) -> str:
        row = self.db.execute(
            "SELECT value FROM kv WHERE key='narrative'"
        ).fetchone()
        return row["value"] if row else ""

    @_locked

    def set_narrative(self, text: str) -> None:
        with self.db:
            self.db.execute(
                "INSERT OR REPLACE INTO kv(key, value) "
                "VALUES ('narrative', ?)",
                (text,),
            )

    @_locked

    def narrative_updated_at(self) -> str:
        row = self.db.execute(
            "SELECT value FROM kv WHERE key='narrative_updated_at'"
        ).fetchone()
        return row["value"] if row else ""

    @_locked

    def set_narrative_updated_at(self, ts: str) -> None:
        with self.db:
            self.db.execute(
                "INSERT OR REPLACE INTO kv(key, value) "
                "VALUES ('narrative_updated_at', ?)",
                (ts,),
            )

    # ---------- quiz history (P4) ----------

    @_locked

    def record_quiz(self, mode: str, words: list[str], quiz: dict) -> int:
        with self.db:
            cur = self.db.execute(
                "INSERT INTO quiz_history(created_at, mode, words_json, "
                "quiz_json) VALUES (?, ?, ?, ?)",
                (
                    _now(),
                    mode,
                    json.dumps(words, ensure_ascii=False),
                    json.dumps(quiz, ensure_ascii=False),
                ),
            )
            return cur.lastrowid

    @_locked

    def mark_quiz_submitted(self, quiz_id: int, score: dict) -> None:
        with self.db:
            self.db.execute(
                "UPDATE quiz_history SET submitted=1, score_json=? "
                "WHERE quiz_id=?",
                (json.dumps(score, ensure_ascii=False), quiz_id),
            )

    @_locked

    def get_quiz(self, quiz_id: int) -> Optional[dict]:
        row = self.db.execute(
            "SELECT quiz_json FROM quiz_history WHERE quiz_id=?",
            (quiz_id,),
        ).fetchone()

        if not row:
            return None

        try:
            return json.loads(row["quiz_json"])
        except json.JSONDecodeError:
            return None

    @_locked

    def list_quizzes(self, limit: int = 10) -> list[dict]:
        rows = self.db.execute(
            "SELECT quiz_id, created_at, mode, words_json, submitted, "
            "score_json FROM quiz_history "
            "ORDER BY quiz_id DESC LIMIT ?",
            (limit,),
        ).fetchall()
        return [
            {
                "quiz_id": r["quiz_id"],
                "created_at": r["created_at"],
                "mode": r["mode"],
                "words": json.loads(r["words_json"]),
                "submitted": bool(r["submitted"]),
                "score": json.loads(r["score_json"]) if r["score_json"] else None,
            }
            for r in rows
        ]

    @_locked

    def recent_quiz_words(self, count: int = 3) -> set[str]:
        """Words used in the most recent quizzes - excluded from new
        quizzes so generation doesn't repeat itself."""
        rows = self.db.execute(
            "SELECT words_json FROM quiz_history "
            "ORDER BY quiz_id DESC LIMIT ?",
            (count,),
        ).fetchall()
        used: set[str] = set()

        for r in rows:
            used.update(json.loads(r["words_json"]))

        return used
