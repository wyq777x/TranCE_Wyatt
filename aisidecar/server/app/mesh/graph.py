"""Assemble the word-mesh graph from both layers.

Node/edge model (consumed by the ECharts force graph):

    nodes: [{id, label, type, meaning, detail}]
        type: center | prefix | root | suffix | synonym | antonym |
              related | family
    edges: [{source, target, relation}]
        relation: has_prefix | has_root | has_suffix | synonym |
                  antonym | related | family
"""

from __future__ import annotations

from .decompose import Decomposition, Morpheme, decompose, family_for
from .llm_mesh import MeshLlmCache, generate_word_mesh


def _morpheme_node(m: Morpheme) -> dict:
    return {
        "id": f"{m.kind}:{m.text}",
        "label": m.text,
        "type": m.kind,
        "meaning": m.meaning,
        "detail": (f"词源：{m.origin}" if m.origin else "")
        + (f" · 词族：{', '.join(m.examples[:5])}" if m.examples else ""),
    }


def build_graph(
    word: str,
    llm_data: dict | None,
    family_enabled: bool = True,
) -> tuple[list[dict], list[dict], Decomposition]:
    decomp = decompose(word)

    nodes: list[dict] = [
        {
            "id": "center",
            "label": word.lower(),
            "type": "center",
            "meaning": (llm_data or {}).get("definition", ""),
            "detail": "",
        }
    ]
    edges: list[dict] = []

    relation_by_kind = {"prefix": "has_prefix", "root": "has_root",
                        "suffix": "has_suffix"}

    for morpheme in decomp.all_morphemes():
        nodes.append(_morpheme_node(morpheme))
        edges.append({
            "source": "center",
            "target": f"{morpheme.kind}:{morpheme.text}",
            "relation": relation_by_kind[morpheme.kind],
        })

        if family_enabled:
            for fam in family_for(morpheme, word)[:4]:
                fam_id = f"family:{morpheme.kind}:{fam}"

                nodes.append({
                    "id": fam_id,
                    "label": fam,
                    "type": "family",
                    "meaning": f"共享{morpheme.kind} {morpheme.text}"
                               f"（{morpheme.meaning}）",
                    "detail": "",
                })
                edges.append({
                    "source": f"{morpheme.kind}:{morpheme.text}",
                    "target": fam_id,
                    "relation": "family",
                })

    if llm_data:
        relation_by_section = {
            "synonyms": "synonym",
            "antonyms": "antonym",
            "related": "related",
        }

        for section, node_type in (
            ("synonyms", "synonym"),
            ("antonyms", "antonym"),
            ("related", "related"),
        ):
            for item in llm_data.get(section, []):
                w = item.get("word", "")

                if not w:
                    continue

                node_id = f"{node_type}:{w.lower()}"

                if any(n["id"] == node_id for n in nodes):
                    continue

                nodes.append({
                    "id": node_id,
                    "label": w,
                    "type": node_type,
                    "meaning": item.get("gloss", ""),
                    "detail": "",
                })
                edges.append({
                    "source": "center",
                    "target": node_id,
                    "relation": relation_by_section[section],
                })

    return nodes, edges, decomp
