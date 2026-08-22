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

import sqlite3
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
"""

# mastery thresholds
WEAK_MASTERY = 0.45
WEAK_MIN_WRONG = 2
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
    def __init__(self, db_path: Path) -> None:
        db_path.parent.mkdir(parents=True, exist_ok=True)
        self.db = sqlite3.connect(db_path)
        self.db.row_factory = sqlite3.Row
        self.db.executescript(SCHEMA)
        self.db.commit()

    # ---------- low-level helpers ----------

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

    def weak_words(self, limit: int = 30) -> list[WordStats]:
        rows = self.db.execute(
            "SELECT * FROM word_mastery "
            "WHERE mastery < ? OR wrong_count >= ? "
            "ORDER BY wrong_count DESC, mastery ASC LIMIT ?",
            (WEAK_MASTERY, WEAK_MIN_WRONG, limit),
        ).fetchall()
        return [self._to_stats(r) for r in rows]

    def all_words(self, limit: int = 500) -> list[WordStats]:
        rows = self.db.execute(
            "SELECT * FROM word_mastery ORDER BY last_seen DESC LIMIT ?",
            (limit,),
        ).fetchall()
        return [self._to_stats(r) for r in rows]

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

    def get_narrative(self) -> str:
        row = self.db.execute(
            "SELECT value FROM kv WHERE key='narrative'"
        ).fetchone()
        return row["value"] if row else ""

    def set_narrative(self, text: str) -> None:
        with self.db:
            self.db.execute(
                "INSERT OR REPLACE INTO kv(key, value) "
                "VALUES ('narrative', ?)",
                (text,),
            )

    def narrative_updated_at(self) -> str:
        row = self.db.execute(
            "SELECT value FROM kv WHERE key='narrative_updated_at'"
        ).fetchone()
        return row["value"] if row else ""

    def set_narrative_updated_at(self, ts: str) -> None:
        with self.db:
            self.db.execute(
                "INSERT OR REPLACE INTO kv(key, value) "
                "VALUES ('narrative_updated_at', ?)",
                (ts,),
            )
