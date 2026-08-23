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
from ..llm import client_for, current_provider
from ..mcp_manager import McpManager, McpPool

router = APIRouter()

MAX_TOOL_ROUNDS = 5


class ChatMessage(BaseModel):
    role: str  # "system" | "user" | "assistant" | "tool"
    content: str


class ChatRequest(BaseModel):
    messages: List[ChatMessage]
    temperature: Optional[float] = None
    use_memory: bool = True
    use_tools: bool = False


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

    mcp_pool: Optional[McpPool] = None

    if payload.use_tools:
        manager = McpManager(request.app.state.config)
        mcp_pool = McpPool(manager)
        await mcp_pool.__aenter__()

        if not mcp_pool.has_tools:
            await mcp_pool.__aexit__(None, None, None)
            mcp_pool = None

    async def event_stream():
        try:
            if mcp_pool is not None:
                # Agent loop with tools: non-streaming rounds (tool-call
                # assembly in streaming mode is provider-inconsistent),
                # final answer delivered as one delta to keep the SSE
                # contract identical for the web UI.
                provider = current_provider()
                client = client_for(provider)
                tools = await mcp_pool.openai_tools()
                final_text = ""

                for _round in range(MAX_TOOL_ROUNDS + 1):
                    response = await client.chat.completions.create(
                        model=provider.chat_model,
                        messages=messages,
                        temperature=payload.temperature,
                        tools=tools,
                    )
                    message = response.choices[0].message

                    if not message.tool_calls:
                        final_text = message.content or ""
                        break

                    messages.append({
                        "role": "assistant",
                        "content": message.content or "",
                        "tool_calls": [
                            {
                                "id": tc.id,
                                "type": "function",
                                "function": {
                                    "name": tc.function.name,
                                    "arguments": tc.function.arguments,
                                },
                            }
                            for tc in message.tool_calls
                        ],
                    })

                    for tool_call in message.tool_calls:
                        try:
                            arguments = json.loads(
                                tool_call.function.arguments or "{}"
                            )
                        except json.JSONDecodeError:
                            arguments = {}

                        result = await mcp_pool.call(
                            tool_call.function.name, arguments
                        )
                        messages.append({
                            "role": "tool",
                            "tool_call_id": tool_call.id,
                            "content": result,
                        })
                else:
                    final_text = "（工具调用轮次已达上限，未得出结论）"

                if final_text:
                    yield _sse("delta", {"text": final_text})

                yield _sse("done", {})
            else:
                async for delta in llm.stream_chat(
                    messages, temperature=payload.temperature
                ):
                    yield _sse("delta", {"text": delta})

                yield _sse("done", {})
        except llm.NoProviderError as exc:
            yield _sse("error", {"message": str(exc)})
        except Exception as exc:  # provider/network errors reach the UI here
            yield _sse("error", {"message": f"{type(exc).__name__}: {exc}"})
        finally:
            if mcp_pool is not None:
                await mcp_pool.__aexit__(None, None, None)

    return StreamingResponse(
        event_stream(),
        media_type="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "X-Accel-Buffering": "no",
        },
    )
