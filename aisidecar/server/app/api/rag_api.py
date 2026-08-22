"""Native RAG endpoints.

GET  /api/rag/status           corpus stats + build progress
POST /api/rag/build            start (background) corpus build
POST /api/lookup/concept       fuzzy concept reverse lookup
POST /api/lookup/scene         scenario expression search
"""

from __future__ import annotations

import asyncio

from fastapi import APIRouter, HTTPException, Request
from pydantic import BaseModel

from ..llm import NoProviderError, client_for, current_provider
from ..rag import CorpusStore, HybridSearcher, build_corpus, build_state

router = APIRouter()

CORPUS_FILENAME = "corpus.db"


def _store(request: Request) -> CorpusStore:
    store = getattr(request.app.state, "corpus_store", None)

    if store is None:
        store = CorpusStore(
            request.app.state.config.data_dir / CORPUS_FILENAME
        )
        request.app.state.corpus_store = store

    return store


def _searcher(request: Request) -> HybridSearcher:
    searcher = getattr(request.app.state, "corpus_searcher", None)

    if searcher is None:
        searcher = HybridSearcher(_store(request))
        request.app.state.corpus_searcher = searcher

    return searcher


# ---------------------------------------------------------------- status


@router.get("/api/rag/status")
def rag_status(request: Request) -> dict:
    return {
        **_store(request).stats(),
        "building": build_state.running,
        "progress": round(build_state.progress, 3),
        "stage": build_state.stage,
        "done": build_state.done,
        "total": build_state.total,
        "error": build_state.error,
    }


class BuildPayload(BaseModel):
    top_n: int = 30000


@router.post("/api/rag/build")
async def rag_build(payload: BuildPayload, request: Request) -> dict:
    if build_state.running:
        return {"started": False, "reason": "build already running"}

    store = _store(request)
    top_n = max(100, min(payload.top_n, 100000))

    async def run() -> None:
        try:
            await build_corpus(store, top_n)
        except Exception:  # recorded in build_state.error
            pass

    asyncio.create_task(run())
    # give the task a beat so /status reflects "building" immediately
    await asyncio.sleep(0)
    return {"started": True}


# ---------------------------------------------------------------- lookups


class ConceptPayload(BaseModel):
    query: str
    top_k: int = 10
    refine: bool = False


REFINE_PROMPT = """\
用户想找一个英文词，其语义描述为："{query}"

候选词表（词 | 中文释义 | 词性）：
{candidates}

从候选中挑出最符合描述的 3-8 个词，并给出：每个词为何贴合（一句话，中文）、\
一个简短例句。用 markdown 列表输出。若候选都不贴切，说明并给出你认为更合适\
的词（如有把握）。不要编造候选之外的词义。\
"""


@router.post("/api/lookup/concept")
async def lookup_concept(payload: ConceptPayload, request: Request) -> dict:
    if not payload.query.strip():
        raise HTTPException(status_code=400, detail="empty query")

    hits = _searcher(request).search(
        payload.query.strip(), top_k=payload.top_k, kind="dict"
    )

    result: dict = {
        "query": payload.query,
        "results": [h.__dict__ for h in hits],
        "refinement": None,
    }

    if payload.refine and hits:
        try:
            provider = current_provider()
            client = client_for(provider)
            candidates = "\n".join(
                f"- {h.word} | {h.translation} | {h.note}" for h in hits[:15]
            )
            response = await client.chat.completions.create(
                model=provider.chat_model,
                messages=[
                    {
                        "role": "user",
                        "content": REFINE_PROMPT.format(
                            query=payload.query, candidates=candidates
                        ),
                    }
                ],
                temperature=0.2,
            )
            result["refinement"] = (
                response.choices[0].message.content or ""
            ).strip()
        except NoProviderError as exc:
            result["refinement_error"] = str(exc)
        except Exception as exc:
            result["refinement_error"] = f"{type(exc).__name__}: {exc}"

    return result


class ScenePayload(BaseModel):
    query: str = ""
    scene: str = "business_email"  # business_email | academic | daily
    top_k: int = 15


@router.post("/api/lookup/scene")
def lookup_scene(payload: ScenePayload, request: Request) -> dict:
    from ..rag.scenes_seed import SCENES

    if payload.scene not in SCENES:
        raise HTTPException(
            status_code=400,
            detail=f"unknown scene, expected one of {list(SCENES)}",
        )

    hits = _searcher(request).search(
        payload.query.strip(),
        top_k=payload.top_k,
        kind="scene",
        scene=payload.scene,
    )
    return {
        "query": payload.query,
        "scene": payload.scene,
        "scene_label": SCENES[payload.scene],
        "results": [h.__dict__ for h in hits],
    }
