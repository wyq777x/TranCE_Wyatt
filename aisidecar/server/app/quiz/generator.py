"""Quiz generation: word selection, structured generation, grading.

Two modes:
  cloze  a short story with N blanks; each blank offers 4 options
  story  a readable story highlighting the target words (no grading)

The LLM always returns structured JSON; a strict sanitizer repairs or
rejects malformed output. LightRAG context (when available) is merged
into the prompt to keep thematic continuity with past quizzes and the
learner profile.
"""

from __future__ import annotations

import json
import re
from typing import Optional

from ..learner import LearnerStore

CLOZE_PROMPT = """\
为英语学习者生成一篇完形填空短文。只输出一个 JSON 对象，不要任何其他文字。

目标词（必须每个都用上，各挖一个空）：{words}
学习者画像：{narrative}
{context_block}要求：
- passage: 120-180 词的英文小故事，情节自然，把每个目标词的位置替换为
  占位符 {{{{1}}}}、{{{{2}}}}…（数字与 items.index 对应）
- items: 每空给出 index、word（正确答案）、options（4 个选项，含正确
  答案，顺序打乱，干扰项为易混词或同话题词）、explanation（中文，说明
  为什么该词贴合语境）
- glossary: 目标词的中文速览
- 故事主题与学习者画像匹配（如画像提到偏好职场场景就用职场故事）

输出格式：
{{"title": "…", "passage": "…", "items": [{{"index": 1, "word": "…", \
"options": ["…","…","…","…"], "explanation": "…"}}], "glossary": \
[{{"word": "…", "meaning": "…"}}]}}\
"""

STORY_PROMPT = """\
为英语学习者写一篇英文短故事，自然地融入目标词。只输出一个 JSON 对象。

目标词：{words}
学习者画像：{narrative}
{context_block}要求：
- passage: 150-250 词英文故事，目标词原样出现（前端会高亮）
- glossary: 每个目标词在本故事语境中的中文含义一句话
- 主题与画像匹配，避免与学习者的既往故事重复

输出格式：
{{"title": "…", "passage": "…", "glossary": [{{"word": "…", \
"meaning": "…"}}]}}\
"""


def pick_words(
    learner: LearnerStore,
    count: int,
    requested: Optional[list[str]] = None,
) -> list[str]:
    """Weak words first (excluding recently quizzed ones), falling back
    to the lowest-mastery known words. When the recent-exclusion leaves
    nothing (small vocabulary), it is relaxed rather than failing."""
    if requested:
        return [w.lower() for w in requested][:count]

    def candidates(exclude: set[str]) -> list:
        picked = [
            w
            for w in learner.weak_words(100)
            if w.word.lower() not in exclude
        ]

        if len(picked) < count:
            seen = {w.word.lower() for w in picked}

            for w in learner.all_words(300):
                if w.word.lower() not in exclude and w.word.lower() not in seen:
                    picked.append(w)
                    seen.add(w.word.lower())

                if len(picked) >= count:
                    break

        return picked[:count]

    picked = candidates(learner.recent_quiz_words(3))

    if not picked:
        # tiny vocabulary: every word was recently quizzed - allow reuse
        picked = candidates(set())

    return [w.word for w in picked]


def extract_json(text: str) -> dict:
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


def build_prompt(
    mode: str,
    words: list[str],
    narrative: str,
    rag_context: str,
) -> str:
    context_block = (
        f"学习者既往学习语境（用于主题延续、避免重复）：\n{rag_context}\n"
        if rag_context
        else ""
    )

    if mode == "story":
        return STORY_PROMPT.format(
            words=", ".join(words),
            narrative=narrative or "（暂无）",
            context_block=context_block,
        )

    return CLOZE_PROMPT.format(
        words=", ".join(words),
        narrative=narrative or "（暂无）",
        context_block=context_block,
    )


def sanitize_cloze(raw: dict, words: list[str]) -> dict:
    """Validate/repair the generated cloze quiz; raises on hopeless
    output."""
    passage = str(raw.get("passage", "")).strip()
    items_raw = raw.get("items", [])
    title = str(raw.get("title", "")).strip() or "Cloze"

    if not passage or not items_raw:
        raise ValueError("quiz missing passage or items")

    # original placeholder ids in order of appearance; these map to the
    # renumbered 1..n so the passage and items always agree
    orig_placeholders = re.findall(r"\{(\d+)\}", passage)
    renumber: dict[str, int] = {
        orig: i + 1 for i, orig in enumerate(orig_placeholders)
    }

    items: list[dict] = []

    for item in items_raw:
        orig_index = str(int(item.get("index", 0)))
        word = str(item.get("word", "")).strip().lower()

        if not word or word not in [w.lower() for w in words]:
            continue

        if orig_index not in renumber:
            continue

        options = [
            str(o).strip() for o in item.get("options", []) if str(o).strip()
        ]

        # exactly 4 options containing the answer, answer not always first
        if word not in [o.lower() for o in options]:
            options.append(word)

        options = options[:4]

        if len(options) < 4:
            continue

        items.append({
            "index": renumber[orig_index],
            "word": word,
            "options": options,
            "explanation": str(item.get("explanation", "")),
        })

    if not items:
        raise ValueError("no valid items referencing target words")

    # rewrite placeholders with the renumbered ids
    passage = re.sub(
        r"\{(\d+)\}",
        lambda m: "{" + str(renumber[m.group(1)]) + "}",
        passage,
    )

    # drop items whose slot got claimed by a duplicate index
    items = [i for i in items if passage.count("{" + str(i["index"]) + "}") == 1]
    items.sort(key=lambda i: i["index"])

    glossary = [
        {"word": str(g.get("word", "")), "meaning": str(g.get("meaning", ""))}
        for g in raw.get("glossary", [])
        if g.get("word")
    ]

    return {
        "type": "cloze",
        "title": title,
        "passage": passage,
        "items": items,
        "glossary": glossary,
        "target_words": [i["word"] for i in items],
    }


def sanitize_story(raw: dict, words: list[str]) -> dict:
    passage = str(raw.get("passage", "")).strip()
    title = str(raw.get("title", "")).strip() or "Story"

    if len(passage) < 120:
        raise ValueError("story too short")

    glossary = [
        {"word": str(g.get("word", "")), "meaning": str(g.get("meaning", ""))}
        for g in raw.get("glossary", [])
        if g.get("word")
    ]

    found = [
        w for w in words if re.search(rf"\b{re.escape(w)}\b", passage, re.I)
    ]

    return {
        "type": "story",
        "title": title,
        "passage": passage,
        "items": [],
        "glossary": glossary,
        # fall back to the requested words when none made it into the
        # passage verbatim (inflected forms etc.)
        "target_words": found or list(words),
    }


def grade_cloze(quiz: dict, answers: dict[str, str]) -> dict:
    """answers: {"<index>": "<chosen option>"}. Returns per-word results."""
    results = []

    for item in quiz["items"]:
        chosen = (answers.get(str(item["index"]), "") or "").strip()
        correct = chosen.lower() == item["word"].lower()
        results.append({
            "index": item["index"],
            "word": item["word"],
            "chosen": chosen,
            "correct": correct,
            "explanation": item["explanation"],
        })

    return {
        "results": results,
        "correct_count": sum(1 for r in results if r["correct"]),
        "total": len(results),
    }
