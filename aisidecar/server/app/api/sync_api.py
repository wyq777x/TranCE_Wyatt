"""Learner-model synchronization endpoints.

POST /api/sync/snapshot  full state push from the Qt host (startup)
POST /api/sync/event     one incremental learning-behaviour event
GET  /api/sync/weak      weak-word list (debug / UI)
"""

from __future__ import annotations

from fastapi import APIRouter, Request
from pydantic import BaseModel

from ..learner_manager import get_current_learner

router = APIRouter()


class SnapshotPayload(BaseModel):
    vocabulary_mastered: list[str] = []
    vocabulary_learning: list[str] = []
    favorites: list[str] = []
    recite_history: list[str] = []
    search_history: list[str] = []


class EventPayload(BaseModel):
    type: str  # quiz_answer | lookup | recite | favorite | word_status
    word: str
    correct: bool | None = None
    favorite: bool | None = None
    status: int | None = None


@router.post("/api/sync/snapshot")
def sync_snapshot(payload: SnapshotPayload, request: Request) -> dict:
    learner = get_current_learner(request.app.state.config)
    learner.apply_snapshot(payload.model_dump())
    return {"ok": True, "stats": learner.stats()}

@router.post("/api/sync/event")
def sync_event(payload: EventPayload, request: Request) -> dict:
    learner = get_current_learner(request.app.state.config)
    learner.apply_event(
        payload.type,
        payload.word,
        correct=payload.correct,
        favorite=payload.favorite,
        status=payload.status,
    )
    return {"ok": True}


@router.get("/api/sync/weak")
def weak_words(request: Request, limit: int = 30) -> dict:
    learner = get_current_learner(request.app.state.config)
    return {
        "words": [w.__dict__ for w in learner.weak_words(limit)],
        "stats": learner.stats(),
    }
