from typing import Dict, Optional
from dataclasses import dataclass, field
import time


@dataclass
class Session:
    session_id: str
    created_at: float = field(default_factory=time.time)
    last_seen: float = field(default_factory=time.time)
    metadata: Dict = field(default_factory=dict)


class SessionManager:
    """Very small in-memory session manager that "allows all sessions".

    This is a scaffold for the full pool manager. It accepts any session
    registration and tracks health timestamps. It is intentionally simple
    for initial integration and testing.
    """

    def __init__(self):
        self._sessions: Dict[str, Session] = {}

    def register(self, session_id: str, metadata: Optional[Dict] = None) -> Session:
        s = Session(session_id=session_id, metadata=metadata or {})
        self._sessions[session_id] = s
        return s

    def touch(self, session_id: str) -> Optional[Session]:
        s = self._sessions.get(session_id)
        if s:
            s.last_seen = time.time()
        return s

    def unregister(self, session_id: str) -> bool:
        return self._sessions.pop(session_id, None) is not None

    def list_sessions(self):
        return list(self._sessions.values())

    def get(self, session_id: str) -> Optional[Session]:
        return self._sessions.get(session_id)
