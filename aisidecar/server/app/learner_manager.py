"""LearnerStore lifecycle keyed by the logged-in user."""

from __future__ import annotations

from pathlib import Path

from .config import Config
from .learner import LearnerStore
from .session import get_session

_stores: dict[str, LearnerStore] = {}


def get_current_learner(config: Config) -> LearnerStore:
    """Return (creating if needed) the learner store of the active user."""
    session = get_session()

    if not session.user_uuid:
        raise RuntimeError("No user session pushed yet")

    store = _stores.get(session.user_uuid)

    if store is None:
        db_path: Path = (
            config.data_dir / "users" / session.user_uuid / "learner.db"
        )
        store = LearnerStore(db_path)
        _stores[session.user_uuid] = store

    return store
