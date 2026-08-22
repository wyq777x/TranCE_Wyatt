"""OpenAI-compatible chat client wrapper."""

from __future__ import annotations

from typing import AsyncIterator, List, Optional

from openai import AsyncOpenAI

from .session import ProviderConfig, get_session


class NoProviderError(RuntimeError):
    pass


def client_for(provider: ProviderConfig) -> AsyncOpenAI:
    return AsyncOpenAI(base_url=provider.base_url, api_key=provider.api_key)


def current_provider() -> ProviderConfig:
    provider = get_session().provider
    if not provider or not provider.base_url or not provider.api_key:
        raise NoProviderError(
            "No AI provider configured. Add one in TranCE Settings."
        )
    return provider


async def stream_chat(
    messages: List[dict],
    temperature: Optional[float] = None,
) -> AsyncIterator[str]:
    """Yield assistant text deltas from the configured provider."""
    provider = current_provider()
    client = client_for(provider)

    kwargs: dict = {
        "model": provider.chat_model,
        "messages": messages,
        "stream": True,
    }

    if temperature is not None:
        kwargs["temperature"] = temperature

    stream = await client.chat.completions.create(**kwargs)

    async for chunk in stream:
        if not chunk.choices:
            continue

        delta = chunk.choices[0].delta

        if delta and delta.content:
            yield delta.content
