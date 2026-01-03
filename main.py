from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
import uvicorn
from session_manager import SessionManager
from typing import Dict

app = FastAPI(title="Tripplex Core (scaffold)")
manager = SessionManager()


class ConnectReq(BaseModel):
    session_id: str
    metadata: Dict = {}


@app.post("/connect")
async def connect(req: ConnectReq):
    s = manager.register(req.session_id, req.metadata)
    return {"session_id": s.session_id, "created_at": s.created_at}


@app.post("/disconnect")
async def disconnect(req: ConnectReq):
    ok = manager.unregister(req.session_id)
    return {"ok": ok}


@app.get("/sessions")
async def sessions():
    return [{"session_id": s.session_id, "last_seen": s.last_seen} for s in manager.list_sessions()]


@app.websocket("/ws/{session_id}")
async def websocket_endpoint(websocket: WebSocket, session_id: str):
    await websocket.accept()
    manager.touch(session_id)
    try:
        while True:
            data = await websocket.receive_text()
            manager.touch(session_id)
            # Echo back — in real Core this would route messages from DLLs and brokers
            await websocket.send_text(f"echo: {data}")
    except WebSocketDisconnect:
        manager.unregister(session_id)


if __name__ == "__main__":
    uvicorn.run("main:app", host="127.0.0.1", port=8000, log_level="info")
