"""Quiz endpoints (P4).

POST /api/quiz/generate   weakness-targeted cloze or story generation
POST /api/quiz/submit     grade answers and feed the learner model
GET  /api/quiz/history    recent quizzes with scores
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Request
from pydantic import BaseModel

from ..learner_manager import get_current_learner
from ..llm import NoProviderError, client_for, current_provider
from ..quiz import (
    build_prompt,
    extract_json,
    grade_cloze,
    pick_words,
    sanitize_cloze,
    sanitize_story,
)
from ..quiz.lightrag_layer import QuizRagLayer

router = APIRouter()


def _rag(request: Request) -> QuizRagLayer:
    layer = getattr(request.app.state, "quiz_rag", None)

    if layer is None:
        layer = QuizRagLayer(request.app.state.config)
        request.app.state.quiz_rag = layer

    return layer


class GeneratePayload(BaseModel):
    mode: str = "cloze"  # cloze | story
    count: int = 4
    words: list[str] = []  # manual selection; empty = auto from weaknesses


@router.post("/api/quiz/generate")
async def quiz_generate(payload: GeneratePayload, request: Request) -> dict:
    if payload.mode not in ("cloze", "story"):
        raise HTTPException(status_code=400, detail="mode must be cloze|story")

    try:
        provider = current_provider()
    except NoProviderError as exc:
        raise HTTPException(status_code=409, detail=str(exc)) from exc

    learner = get_current_learner(request.app.state.config)
    count = max(2, min(payload.count, 6))
    words = pick_words(learner, count, payload.words or None)

    if not words:
        raise HTTPException(
            status_code=409,
            detail="no vocabulary to quiz from - look up/recite words first",
        )

    # LightRAG enhancement layer (best-effort, degrades to "")
    rag = _rag(request)
    rag_context = ""

    try:
        await rag.ensure_ingested(learner)
        rag_context = await rag.context_for(words)
    except Exception:
        rag_context = ""

    client = client_for(provider)
    response = await client.chat.completions.create(
        model=provider.chat_model,
        messages=[
            {
                "role": "user",
                "content": build_prompt(
                    payload.mode,
                    words,
                    learner.get_narrative(),
                    rag_context,
                ),
            }
        ],
        temperature=0.7,
    )

    try:
        raw = extract_json(response.choices[0].message.content or "")

        quiz = (
            sanitize_cloze(raw, words)
            if payload.mode == "cloze"
            else sanitize_story(raw, words)
        )
    except (ValueError, KeyError) as exc:
        raise HTTPException(
            status_code=502,
            detail=f"malformed quiz from provider: {exc}",
        ) from exc

    quiz_id = learner.record_quiz(payload.mode, quiz["target_words"], quiz)

    return {
        "quiz_id": quiz_id,
        "quiz": quiz,
        "rag_used": bool(rag_context),
    }


class SubmitPayload(BaseModel):
    quiz_id: int
    answers: dict[str, str]  # {"<item index>": "<chosen option>"}


@router.post("/api/quiz/submit")
def quiz_submit(payload: SubmitPayload, request: Request) -> dict:
    learner = get_current_learner(request.app.state.config)
    quiz = learner.get_quiz(payload.quiz_id)

    if quiz is None:
        raise HTTPException(status_code=404, detail="quiz not found")

    if quiz.get("type") != "cloze":
        raise HTTPException(status_code=400, detail="story quizzes are not graded")

    score = grade_cloze(quiz, payload.answers)

    # feed the learner model - this is the mastery loop closing
    for result in score["results"]:
        learner.apply_event(
            "quiz_answer", result["word"], correct=result["correct"]
        )

    learner.mark_quiz_submitted(payload.quiz_id, score)

    return score


@router.get("/api/quiz/history")
def quiz_history(request: Request, limit: int = 10) -> dict:
    learner = get_current_learner(request.app.state.config)
    return {"quizzes": learner.list_quizzes(limit)}
