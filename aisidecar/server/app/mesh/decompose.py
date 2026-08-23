"""Morphological decomposer: word -> prefixes + core + suffixes + roots.

Greedy longest-match peeling with plausibility guards (a morpheme only
splits off when enough letters remain to look like a real stem). The
result drives the offline layer of the word mesh: the center word links
to its morphemes, and morphemes link to their example words (family).
"""

from __future__ import annotations

from dataclasses import dataclass, field

from .data_morphemes import PREFIXES, ROOTS, SUFFIXES

MIN_STEM = 3

_PREFIX_TABLE = sorted(PREFIXES, key=lambda p: -len(p[0]))
_SUFFIX_TABLE = sorted(SUFFIXES, key=lambda s: -len(s[0]))
_ROOT_INDEX: dict[str, tuple[str, str, str, list[str]]] = {}

for root, meaning, origin, examples in ROOTS:
    _ROOT_INDEX[root] = (root, meaning, origin, examples)


@dataclass
class Morpheme:
    text: str
    kind: str  # prefix | root | suffix
    meaning: str
    origin: str = ""
    examples: list[str] = field(default_factory=list)


@dataclass
class Decomposition:
    word: str
    prefixes: list[Morpheme] = field(default_factory=list)
    suffixes: list[Morpheme] = field(default_factory=list)
    roots: list[Morpheme] = field(default_factory=list)

    @property
    def is_empty(self) -> bool:
        return not (self.prefixes or self.suffixes or self.roots)

    def all_morphemes(self) -> list[Morpheme]:
        return self.prefixes + self.roots + self.suffixes


def _peel_prefixes(word: str) -> tuple[list[Morpheme], str]:
    found: list[Morpheme] = []
    rest = word

    changed = True

    while changed and len(rest) > MIN_STEM + 1:
        changed = False

        for morph, meaning, examples in _PREFIX_TABLE:
            if rest.startswith(morph) and len(rest) - len(morph) >= MIN_STEM:
                found.append(Morpheme(morph, "prefix", meaning,
                                      examples=list(examples)))
                rest = rest[len(morph):]
                changed = True
                break

    return found, rest


def _peel_suffixes(word: str) -> tuple[list[Morpheme], str]:
    found: list[Morpheme] = []
    rest = word

    changed = True

    while changed and len(rest) > MIN_STEM + 1:
        changed = False

        for morph, meaning, examples in _SUFFIX_TABLE:
            if rest.endswith(morph) and len(rest) - len(morph) >= MIN_STEM:
                found.append(Morpheme(morph, "suffix", meaning,
                                      examples=list(examples)))
                rest = rest[: -len(morph)]
                changed = True
                break

    return found, rest


def _find_roots(stem: str) -> list[Morpheme]:
    """Match roots inside the stem by longest containment ('dict' in
    'predict' but also 'pre' handled as prefix)."""
    hits: list[Morpheme] = []
    ordered = sorted(_ROOT_INDEX.values(), key=lambda r: -len(r[0]))

    for root, meaning, origin, examples in ordered:
        if root in stem:
            hits.append(Morpheme(root, "root", meaning, origin,
                                 examples=list(examples)))

            if len(hits) >= 2:  # keep the graph readable
                break

    return hits


def decompose(word: str) -> Decomposition:
    w = word.strip().lower()

    if not w.isalpha() or len(w) < MIN_STEM + 1:
        return Decomposition(word=w)

    prefixes, rest = _peel_prefixes(w)
    suffixes, stem = _peel_suffixes(rest)

    if len(stem) >= 2:
        roots = _find_roots(stem)
    else:
        roots = []

    return Decomposition(word=w, prefixes=prefixes, suffixes=suffixes,
                         roots=roots)


def family_for(morpheme: Morpheme, exclude: str) -> list[str]:
    """Example words sharing this morpheme, minus the center word."""
    return [w for w in morpheme.examples if w.lower() != exclude.lower()]
