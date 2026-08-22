"""Streaming chat endpoint (SSE).

The web UI owns the conversation state and posts the full message list
each turn. With use_memory enabled (default), the learner's narrative
profile and weak words are injected as system context so answers adapt
to the user's mastery.
"""

from __future__ import annotations

import json
from typing import List, Optional

from fastapi import APIRouter, Request
from fastapi.responses import StreamingResponse
from pydantic import BaseModel

from .. import llm
from ..learner_manager import get_current_learner

router = APIRouter()


class ChatMessage(BaseModel):
    role: str  # "system" | "user" | "assistant"
    content: str


class ChatRequest(BaseModel):
    messages: List[ChatMessage]
    temperature: Optional[float] = None
    use_memory: bool = True


def _build_memory_context(config) -> str:  # noqa: ANN001
    """Assemble learner-memory system context; empty when unavailable."""
    try:
        learner = get_current_learner(config)
        narrative = learner.get_narrative()
        weak = learner.weak_words(10)

        if not narrative and not weak:
            return ""

        parts = ["[学习者记忆上下文 - 个性化教学依据]"]

        if narrative:
            parts.append(f"学习者画像:\n{narrative}")

        if weak:
            parts.append(
                "当前弱项词: "
                + ", ".join(f"{w.word}(掌握度{w.mastery})" for w in weak)
            )

        return "\n\n".join(parts)
    except Exception:
        # Memory is an enhancement, never a hard dependency of chat.
        return ""


def _sse(event: str, data: dict) -> str:
    return f"event: {event}\ndata: {json.dumps(data, ensure_ascii=False)}\n\n"


@router.post("/api/chat")
async def chat(payload: ChatRequest, request: Request) -> StreamingResponse:
    messages = [m.model_dump() for m in payload.messages]

    if payload.use_memory:
        memory_context = _build_memory_context(request.app.state.config)

        if memory_context:
            # merge into the first system message (or prepend one)
            if messages and messages[0]["role"] == "system":
                messages[0]["content"] += "\n\n" + memory_context
            else:
                messages.insert(
                    0, {"role": "system", "content": memory_context}
                )

    async def event_stream():
        try:
            async for delta in llm.stream_chat(
                messages, temperature=payload.temperature
            ):
                yield _sse("delta", {"text": delta})

            yield _sse("done", {})
        except llm.NoProviderError as exc:
            yield _sse("error", {"message": str(exc)})
        except Exception as exc:  # provider/network errors reach the UI here
            yield _sse("error", {"message": f"{type(exc).__name__}: {exc}"})

    return StreamingResponse(
        event_stream(),
        media_type="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "X-Accel-Buffering": "no",
        },
    )
