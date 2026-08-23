"""LightRAG enhancement layer for quiz generation.

Owns a per-user LightRAG workspace (directory name keyed by the
embedding model, so switching models starts a clean store). Provides:
  - ensure_ingested: narrative (on change) + past quiz passages (new)
  - context_for: retrieval context for the generation prompt

Every failure degrades to "no context" - the quiz generator works
without this layer whenever LightRAG or embeddings are unavailable.
"""

from __future__ import annotations

import hashlib
import json
import logging
from pathlib import Path

from ..config import Config
from ..llm import NoProviderError, current_provider
from ..session import get_session

logger = logging.getLogger("trance.quiz.rag")


class QuizRagLayer:
    """Lazily-initialized; one live instance for the active user."""

    def __init__(self, config: Config) -> None:
        self.config = config
        self._rag = None
        self._user = ""
        self._embedding_model = ""
        self._ingested_quiz_ids: set[int] = set()
        self._ingested_narrative_at = ""
        self._failed = False

    # ---------- lifecycle ----------

    def reset(self) -> None:
        self._rag = None
        self._user = ""
        self._embedding_model = ""
        self._ingested_quiz_ids = set()
        self._ingested_narrative_at = ""
        self._failed = False

    @property
    def available(self) -> bool:
        return self._rag is not None and not self._failed

    def _provider(self):
        provider = current_provider()  # raises NoProviderError

        if not provider.embedding_model:
            raise NoProviderError(
                "embedding model not configured - LightRAG disabled"
            )

        return provider

    def _ensure_rag(self):
        """(Re)create the LightRAG instance for the current user/model."""
        provider = self._provider()
        session = get_session()

        if (
            self._rag is not None
            and self._user == session.user_uuid
            and self._embedding_model == provider.embedding_model
        ):
            return self._rag

        from lightrag import LightRAG
        from lightrag.llm.openai import openai_complete_if_cache, openai_embed
        from lightrag.utils import EmbeddingFunc

        # dimension unknown until first embed; probe once
        probe = openai_embed(
            ["dimension probe"],
            model=provider.embedding_model,
            base_url=provider.base_url,
            api_key=provider.api_key,
        )
        dim = len(probe[0])

        model_hash = hashlib.sha1(
            provider.embedding_model.encode()
        ).hexdigest()[:8]
        working_dir = (
            self.config.data_dir
            / "users"
            / session.user_uuid
            / f"lightrag-{model_hash}"
        )
        working_dir.mkdir(parents=True, exist_ok=True)

        self._rag = LightRAG(
            working_dir=str(working_dir),
            llm_model_func=lambda prompt, **kwargs: openai_complete_if_cache(
                model=provider.chat_model,
                prompt=prompt,
                base_url=provider.base_url,
                api_key=provider.api_key,
                **kwargs,
            ),
            embedding_func=EmbeddingFunc(
                embedding_dim=dim,
                max_token_size=8192,
                func=lambda texts: openai_embed(
                    texts=texts,
                    model=provider.embedding_model,
                    base_url=provider.base_url,
                    api_key=provider.api_key,
                ),
            ),
            log_level="WARNING",
        )
        self._user = session.user_uuid
        self._embedding_model = provider.embedding_model
        return self._rag

    # ---------- ingest ----------

    async def ensure_ingested(self, learner) -> None:  # noqa: ANN001
        if self._failed:
            return

        try:
            rag = self._ensure_rag()
        except Exception as exc:
            logger.info("LightRAG unavailable: %s", exc)
            self._failed = True
            return

        # narrative (re-ingest when the profile changed)
        narrative = learner.get_narrative()

        if narrative and learner.narrative_updated_at() != (
            self._ingested_narrative_at
        ):
            try:
                await rag.insert(
                    [f"学习者画像（学习风格与弱项总结）：\n{narrative}"]
                )
                self._ingested_narrative_at = learner.narrative_updated_at()
            except Exception as exc:
                logger.warning("narrative ingest failed: %s", exc)

        # past quiz passages (only the newest, un-ingested ones)
        for q in learner.list_quizzes(limit=30):
            if q["quiz_id"] in self._ingested_quiz_ids or not q["submitted"]:
                continue

            try:
                quiz = learner.get_quiz(q["quiz_id"])
            except Exception:
                continue

            if quiz:
                await rag.insert([
                    f"既往故事《{quiz.get('title', '')}》：\n"
                    f"{quiz.get('passage', '')}"
                ])
                self._ingested_quiz_ids.add(q["quiz_id"])

    # ---------- retrieval ----------

    async def context_for(self, words: list[str]) -> str:
        if self._failed:
            return ""

        try:
            rag = self._ensure_rag()

            from lightrag import QueryParam

            result = await rag.query(
                f"学习者与这些词相关的学习语境与故事主题："
                f"{', '.join(words)}",
                param=QueryParam(mode="hybrid", only_need_context=True),
            )

            # only_need_context returns the raw context block; trim it so
            # it doesn't dominate the generation prompt
            text = str(result)

            if len(text) > 3000:
                text = text[:3000] + "\n…(截断)"

            return text
        except Exception as exc:
            logger.info("LightRAG context retrieval failed: %s", exc)
            return ""
