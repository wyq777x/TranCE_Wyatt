"""In-memory session state pushed by the Qt host after startup.

The host owns credential storage (OS keychain); the resolved provider
config with the plain API key is handed over this local authenticated
channel once per session and never persisted to disk by the sidecar.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional


@dataclass
class ProviderConfig:
    base_url: str
    api_key: str
    chat_model: str
    embedding_model: str = ""


@dataclass
class Session:
    user_uuid: str = ""
    username: str = ""
    language: str = "en_US"
    provider: Optional[ProviderConfig] = None
    extra: dict = field(default_factory=dict)


# Single-user local sidecar: one live session at a time.
_session = Session()

# Read-only path to the host's dictionary DB, used by the RAG corpus
# builder. Kept outside Session because it is host state, not user state.
_dict_db_path = ""


def set_dict_db_path(path: str) -> None:
    global _dict_db_path
    _dict_db_path = path


def get_dict_db_path() -> str:
    return _dict_db_path


def set_session(session: Session) -> None:
    global _session
    _session = session


def get_session() -> Session:
    return _session


def has_provider() -> bool:
    provider = _session.provider
    return bool(
        provider
        and provider.base_url
        and provider.api_key
        and provider.chat_model
    )
