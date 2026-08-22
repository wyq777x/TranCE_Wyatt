"""Learner memory profile (narrative) endpoints.

The narrative is a markdown document the LLM periodically consolidates
from learning behaviour (weak words, quiz patterns). It is injected as
system context into AI features so every answer adapts to the learner.
The user can view and edit it - their edit wins until the next
consolidation.
"""

from __future__ import annotations

import time

from fastapi import APIRouter, HTTPException, Request
from pydantic import BaseModel

from ..learner_manager import get_current_learner
from ..llm import NoProviderError, client_for, current_provider

router = APIRouter()


class NarrativePayload(BaseModel):
    narrative: str


@router.get("/api/memory/profile")
def memory_profile(request: Request) -> dict:
    learner = get_current_learner(request.app.state.config)

    return {
        "narrative": learner.get_narrative(),
        "updated_at": learner.narrative_updated_at(),
        "stats": learner.stats(),
        "weak_words": [w.__dict__ for w in learner.weak_words(30)],
    }


@router.put("/api/memory/profile")
def update_narrative(payload: NarrativePayload, request: Request) -> dict:
    learner = get_current_learner(request.app.state.config)
    learner.set_narrative(payload.narrative)
    learner.set_narrative_updated_at(
        time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime())
    )
    return {"ok": True}


CONSOLIDATE_PROMPT = """\
你是一个语言学习分析助手。根据以下学习者数据，生成一份简洁的学习者画像\
（narrative.md），用于指导后续个性化教学。

要求：
- 使用简体中文，markdown 格式，150-300 字
- 包含：当前水平评估、典型弱项与混淆词、学习行为特征、建议的教学策略
- 若提供了"现有画像"，在其基础上增量更新而非推翻重写
- 事实导向，不编造数据中没有的行为

学习者数据：
{data}

现有画像（可能为空）：
{previous}
"""


@router.post("/api/memory/consolidate")
async def consolidate(request: Request) -> dict:
    learner = get_current_learner(request.app.state.config)

    try:
        provider = current_provider()
    except NoProviderError as exc:
        raise HTTPException(status_code=409, detail=str(exc)) from exc

    stats = learner.stats()
    weak = learner.weak_words(20)

    data = (
        f"词汇总量: {stats['total_words']}, 高掌握: {stats['strong_words']}, "
        f"弱项: {stats['weak_words']}, 累计学习事件: {stats['events']}\n"
        "弱项词（词 / 掌握度 / 答错次数）:\n"
        + "\n".join(
            f"- {w.word} / {w.mastery} / 错{w.wrong_count}次" for w in weak
        )
    )

    client = client_for(provider)
    response = await client.chat.completions.create(
        model=provider.chat_model,
        messages=[
            {
                "role": "user",
                "content": CONSOLIDATE_PROMPT.format(
                    data=data, previous=learner.get_narrative() or "（无）"
                ),
            }
        ],
        temperature=0.3,
    )

    narrative = (response.choices[0].message.content or "").strip()

    if not narrative:
        raise HTTPException(status_code=502, detail="empty consolidation")

    learner.set_narrative(narrative)
    learner.set_narrative_updated_at(
        time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime())
    )

    return {"ok": True, "narrative": narrative}
