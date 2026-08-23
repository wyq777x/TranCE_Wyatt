"""LLM layer of the word mesh: synonyms / antonyms / related words.

Results are cached forever in mesh.db (word knowledge is stable, the
cost is not). Falls back to cache-only when no provider is configured.
"""

from __future__ import annotations

import json
import sqlite3
import threading
import time
from pathlib import Path

from ..llm import NoProviderError, client_for, current_provider

SCHEMA = """
CREATE TABLE IF NOT EXISTS llm_mesh(
    word TEXT PRIMARY KEY,
    json TEXT NOT NULL,
    created_at TEXT NOT NULL
);
"""

MESH_PROMPT = """\
分析英文单词 "{word}"，生成词网联想数据。只输出一个 JSON 对象，\
不要任何其他文字：

{{
  "definition": "简明中文释义（30字内）",
  "synonyms": [{{"word": "同义词", "gloss": "中文区别一句话"}}],
  "antonyms": [{{"word": "反义词", "gloss": "中文说明"}}],
  "related": [{{"word": "联想词（近义场景/派生/搭配）", "gloss": "关系说明"}}]
}}

要求：synonyms 3-5 个、antonyms 0-4 个（没有就不编造）、related 3-6 个；\
全部使用常见词；gloss 用中文，20字内。\
"""


class MeshLlmCache:
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

    def get(self, word: str) -> dict | None:
        with self.lock:
            row = self.db.execute(
                "SELECT json FROM llm_mesh WHERE word=?", (word.lower(),)
            ).fetchone()

        if not row:
            return None

        try:
            return json.loads(row["json"])
        except json.JSONDecodeError:
            return None

    def put(self, word: str, data: dict) -> None:
        with self.lock:
            with self.db:
                self.db.execute(
                    "INSERT OR REPLACE INTO llm_mesh(word, json, created_at) "
                    "VALUES (?, ?, ?)",
                    (
                        word.lower(),
                        json.dumps(data, ensure_ascii=False),
                        time.strftime("%Y-%m-%dT%H:%M:%S"),
                    ),
                )


def _extract_json(text: str) -> dict:
    """LLMs sometimes wrap JSON in fences or prose; find the object."""
    text = text.strip()
    start = text.find("{")

    if start < 0:
        raise ValueError("no JSON object in response")

    depth = 0

    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1

            if depth == 0:
                return json.loads(text[start : i + 1])

    raise ValueError("unbalanced JSON in response")


async def generate_word_mesh(word: str, cache: MeshLlmCache) -> dict:
    """Return cached or freshly generated synonym/antonym data."""
    cached = cache.get(word)

    if cached is not None:
        return cached

    provider = current_provider()  # raises NoProviderError when unset
    client = client_for(provider)
    response = await client.chat.completions.create(
        model=provider.chat_model,
        messages=[{"role": "user", "content": MESH_PROMPT.format(word=word)}],
        temperature=0.2,
    )
    data = _extract_json(response.choices[0].message.content or "")

    # sanitize: keep only expected keys/lists
    clean = {
        "definition": str(data.get("definition", "")),
        "synonyms": [
            {"word": str(x.get("word", "")), "gloss": str(x.get("gloss", ""))}
            for x in data.get("synonyms", [])
            if x.get("word")
        ][:5],
        "antonyms": [
            {"word": str(x.get("word", "")), "gloss": str(x.get("gloss", ""))}
            for x in data.get("antonyms", [])
            if x.get("word")
        ][:4],
        "related": [
            {"word": str(x.get("word", "")), "gloss": str(x.get("gloss", ""))}
            for x in data.get("related", [])
            if x.get("word")
        ][:6],
    }
    cache.put(word, clean)
    return clean
