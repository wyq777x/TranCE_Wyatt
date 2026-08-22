// Sidecar API client.
//
// The one-time bearer token arrives via the page URL (?token=...) and is
// kept in sessionStorage for same-origin API calls.

export interface SessionState {
  username: string;
  language: string;
  provider_configured: boolean;
  chat_model: string;
  embedding_model: string;
}

export function getToken(): string {
  const fromUrl = new URLSearchParams(window.location.search).get("token");
  if (fromUrl) {
    sessionStorage.setItem("trance_ai_token", fromUrl);
    // strip the token from the address bar
    window.history.replaceState({}, "", window.location.pathname);
  }
  return sessionStorage.getItem("trance_ai_token") ?? "";
}

async function request(path: string, init?: RequestInit): Promise<Response> {
  const response = await fetch(path, {
    ...init,
    headers: {
      Authorization: `Bearer ${getToken()}`,
      "Content-Type": "application/json",
      ...(init?.headers ?? {}),
    },
  });
  return response;
}

export async function fetchSessionState(): Promise<SessionState> {
  const response = await request("/api/session/state");
  if (!response.ok) {
    throw new Error(`session state failed: ${response.status}`);
  }
  return response.json();
}

export interface WeakWord {
  word: string;
  mastery: number;
  srs_stage: number;
  correct_count: number;
  wrong_count: number;
  lookups: number;
  recites: number;
  favorite: boolean;
}

export interface MemoryProfile {
  narrative: string;
  updated_at: string;
  stats: {
    total_words: number;
    strong_words: number;
    weak_words: number;
    events: number;
  };
  weak_words: WeakWord[];
}

export async function fetchMemoryProfile(): Promise<MemoryProfile> {
  const response = await request("/api/memory/profile");
  if (!response.ok) {
    throw new Error(`memory profile failed: ${response.status}`);
  }
  return response.json();
}

export async function saveNarrative(narrative: string): Promise<void> {
  const response = await request("/api/memory/profile", {
    method: "PUT",
    body: JSON.stringify({ narrative }),
  });
  if (!response.ok) {
    throw new Error(`save narrative failed: ${response.status}`);
  }
}

// LLM consolidation; needs a configured provider (409 otherwise).
export async function consolidateMemory(): Promise<string> {
  const response = await request("/api/memory/consolidate", {
    method: "POST",
  });
  const body = await response.json().catch(() => ({}));
  if (!response.ok) {
    throw new Error(body.detail ?? `consolidate failed: ${response.status}`);
  }
  return body.narrative as string;
}

// ---------------- native RAG (P2) ----------------

export interface RagStatus {
  built: boolean;
  total: number;
  dict_entries: number;
  scene_entries: number;
  embedded: number;
  embedding_model: string;
  dim: number;
  building: boolean;
  progress: number;
  stage: string;
  done: number;
  total_build: number;
  error: string;
}

export interface LookupHit {
  entry_id: number;
  kind: string;
  word: string;
  translation: string;
  note: string;
  frequency: number;
  scene: string;
  score: number;
  sources: string[];
  vec_distance: number | null;
}

export interface ConceptResult {
  query: string;
  results: LookupHit[];
  refinement: string | null;
  refinement_error?: string;
}

export interface SceneResult {
  query: string;
  scene: string;
  scene_label: string;
  results: LookupHit[];
}

export async function fetchRagStatus(): Promise<RagStatus> {
  const response = await request("/api/rag/status");
  if (!response.ok) {
    throw new Error(`rag status failed: ${response.status}`);
  }
  return response.json();
}

export async function buildRag(topN = 30000): Promise<void> {
  const response = await request("/api/rag/build", {
    method: "POST",
    body: JSON.stringify({ top_n: topN }),
  });
  if (!response.ok) {
    throw new Error(`rag build failed: ${response.status}`);
  }
}

export async function lookupConcept(
  query: string,
  refine = false,
  topK = 10,
): Promise<ConceptResult> {
  const response = await request("/api/lookup/concept", {
    method: "POST",
    body: JSON.stringify({ query, refine, top_k: topK }),
  });
  if (!response.ok) {
    const body = await response.json().catch(() => ({}));
    throw new Error(body.detail ?? `concept lookup failed`);
  }
  return response.json();
}

export async function lookupScene(
  scene: string,
  query: string,
  topK = 30,
): Promise<SceneResult> {
  const response = await request("/api/lookup/scene", {
    method: "POST",
    body: JSON.stringify({ scene, query, top_k: topK }),
  });
  if (!response.ok) {
    const body = await response.json().catch(() => ({}));
    throw new Error(body.detail ?? `scene lookup failed`);
  }
  return response.json();
}

export interface ChatMessage {
  role: "system" | "user" | "assistant";
  content: string;
}

// POST /api/chat and consume the SSE stream, invoking onDelta for each
// text chunk. Returns the full assistant message.
export async function streamChat(
  messages: ChatMessage[],
  onDelta: (text: string) => void,
  useMemory = true,
): Promise<string> {
  const response = await request("/api/chat", {
    method: "POST",
    body: JSON.stringify({ messages, use_memory: useMemory }),
  });

  if (!response.ok || !response.body) {
    throw new Error(`chat failed: ${response.status}`);
  }

  const reader = response.body.getReader();
  const decoder = new TextDecoder();
  let buffer = "";
  let full = "";

  // SSE events separated by blank lines
  for (;;) {
    const { done, value } = await reader.read();
    if (done) break;

    buffer += decoder.decode(value, { stream: true });

    const events = buffer.split("\n\n");
    buffer = events.pop() ?? "";

    for (const raw of events) {
      const lines = raw.split("\n");
      const event = lines
        .find((l) => l.startsWith("event: "))
        ?.slice(7)
        .trim();
      const dataLine = lines
        .find((l) => l.startsWith("data: "))
        ?.slice(6);

      if (event === "delta" && dataLine) {
        const text = JSON.parse(dataLine).text as string;
        full += text;
        onDelta(text);
      } else if (event === "error" && dataLine) {
        throw new Error(JSON.parse(dataLine).message as string);
      }
    }
  }

  return full;
}
