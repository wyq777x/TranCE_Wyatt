"""Word-mesh endpoints.

POST /api/mesh/expand    morphology graph + (cached) LLM synonyms
POST /api/mesh/explain   LLM etymology/memory narrative (markdown)
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Request
from pydantic import BaseModel

from ..llm import NoProviderError, client_for, current_provider
from ..mesh.graph import build_graph
from ..mesh.llm_mesh import MeshLlmCache, generate_word_mesh

router = APIRouter()

MESH_DB = "mesh.db"


def _cache(request: Request) -> MeshLlmCache:
    cache = getattr(request.app.state, "mesh_cache", None)

    if cache is None:
        cache = MeshLlmCache(
            request.app.state.config.data_dir / MESH_DB
        )
        request.app.state.mesh_cache = cache

    return cache


class ExpandPayload(BaseModel):
    word: str
    include_llm: bool = True


@router.post("/api/mesh/expand")
async def mesh_expand(payload: ExpandPayload, request: Request) -> dict:
    word = payload.word.strip().lower()

    if not word.isalpha() or len(word) < 3:
        raise HTTPException(status_code=400, detail="invalid word")

    cache = _cache(request)
    llm_data = None
    llm_error = ""

    if payload.include_llm:
        try:
            llm_data = await generate_word_mesh(word, cache)
        except NoProviderError as exc:
            llm_error = str(exc)
        except Exception as exc:
            llm_error = f"{type(exc).__name__}: {exc}"

    nodes, edges, decomp = build_graph(word, llm_data)

    return {
        "word": word,
        "nodes": nodes,
        "edges": edges,
        "definition": (llm_data or {}).get("definition", ""),
        "morphemes": [
            {"text": m.text, "kind": m.kind, "meaning": m.meaning}
            for m in decomp.all_morphemes()
        ],
        "llm_used": llm_data is not None,
        "llm_error": llm_error,
    }


EXPLAIN_PROMPT = """\
用中文为英语学习者讲解单词 "{word}" 的记忆路径。已知词素分解：
{morphemes}

输出 markdown，包含三个部分：
1. **词源拆解** - 逐个词素讲含义和来源，说明它们如何组合出词义
2. **联想网络** - 结合上述同义/反义/词族关系，给出记忆锚点
3. **一句话记住** - 一条浓缩的记忆口诀

控制在 250 字内，实用优先，不要堆砌辞藻。\
"""


class ExplainPayload(BaseModel):
    word: str
    morphemes: list[dict] = []


@router.post("/api/mesh/explain")
async def mesh_explain(payload: ExplainPayload, request: Request) -> dict:
    word = payload.word.strip().lower()

    if not word:
        raise HTTPException(status_code=400, detail="empty word")

    try:
        provider = current_provider()
    except NoProviderError as exc:
        raise HTTPException(status_code=409, detail=str(exc)) from exc

    morphemes = payload.morphemes or []
    morph_text = (
        "\n".join(
            f"- {m.get('kind', '')} {m.get('text', '')}: "
            f"{m.get('meaning', '')}"
            for m in morphemes
        )
        or "（无词素分解结果）"
    )

    cache = _cache(request)
    llm_data = cache.get(word)

    if llm_data:
        syn = ", ".join(x["word"] for x in llm_data.get("synonyms", []))
        ant = ", ".join(x["word"] for x in llm_data.get("antonyms", []))
        morph_text += f"\n同义词：{syn or '无'}\n反义词：{ant or '无'}"

    client = client_for(provider)
    response = await client.chat.completions.create(
        model=provider.chat_model,
        messages=[
            {
                "role": "user",
                "content": EXPLAIN_PROMPT.format(
                    word=word, morphemes=morph_text
                ),
            }
        ],
        temperature=0.4,
    )

    markdown = (response.choices[0].message.content or "").strip()

    if not markdown:
        raise HTTPException(status_code=502, detail="empty explanation")

    return {"word": word, "markdown": markdown}
