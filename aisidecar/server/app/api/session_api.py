"""Session push endpoint: the Qt host announces the logged-in user and
the resolved provider credentials after the sidecar became healthy."""

from __future__ import annotations

from fastapi import APIRouter
from pydantic import BaseModel

from .. import session as session_store
from ..session import ProviderConfig, Session

router = APIRouter()


class ProviderPayload(BaseModel):
    base_url: str
    api_key: str
    chat_model: str
    embedding_model: str = ""


class SessionPayload(BaseModel):
    user_uuid: str
    username: str
    language: str = "en_US"
    provider: ProviderPayload | None = None


@router.post("/api/session")
def set_session(payload: SessionPayload) -> dict:
    previous = session_store.get_session()

    updated = Session(
        user_uuid=payload.user_uuid,
        username=payload.username,
        language=payload.language,
        provider=(
            ProviderConfig(
                base_url=payload.provider.base_url.rstrip("/"),
                api_key=payload.provider.api_key,
                chat_model=payload.provider.chat_model,
                embedding_model=payload.provider.embedding_model,
            )
            if payload.provider
            else None
        ),
    )

    session_store.set_session(updated)

    provider_changed = (
        previous.provider is None and updated.provider is not None
    ) or (
        previous.provider is not None
        and updated.provider is not None
        and (
            previous.provider.base_url != updated.provider.base_url
            or previous.provider.api_key != updated.provider.api_key
            or previous.provider.chat_model != updated.provider.chat_model
        )
    )

    return {"ok": True, "provider_changed": provider_changed}


@router.get("/api/session/state")
def session_state() -> dict:
    current = session_store.get_session()

    return {
        "username": current.username,
        "language": current.language,
        "provider_configured": session_store.has_provider(),
        # The API key is never echoed back, not even to the local web UI.
        "chat_model": current.provider.chat_model if current.provider else "",
        "embedding_model": (
            current.provider.embedding_model if current.provider else ""
        ),
    }
