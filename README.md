# Tripplex — AI Chart Analyzer (Prototype)

This workspace contains a minimal prototype of the Tripplex mobile app:

- Frontend: Expo React Native app in `frontend`.
- Backend: FastAPI AI analyzer prototype in `backend`.

Quick start (two terminals):

Backend:
```bash
cd backend
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

Frontend (requires Node & Expo CLI):
```bash
cd frontend
npm install
npm run start
```

Notes:
- The analyzer is a light prototype that converts an uploaded chart image into a simplified signal and returns detected support/resistance and simple patterns. It is not production-ready.

Docker & CI
 - Build backend Docker image:

```powershell
cd d:\TrippleX\backend
docker build -t tripplex-backend .
```

 - The repository contains a GitHub Actions workflow at `.github/workflows/ci.yml` that installs dependencies and verifies the backend imports correctly.
 - The repository contains a GitHub Actions workflow at `.github/workflows/ci.yml` that installs dependencies and runs model-backed E2E tests when `OPENAI_API_KEY` and `ENABLE_OPENAI` are set in repository secrets.

Release
-------
To create a release (example):

```bash
git add CHANGELOG.md
git commit -m "chore(release): 0.1.0"
git tag v0.1.0
git push origin master --tags
```

See `RELEASE_CHECKLIST.md` for final release tasks.

# Tripplex scaffold

This workspace contains initial scaffolding for Tripplex components: a FastAPI core that accepts sessions, a TLS echo server for DLL connectivity tests, and a sample MQL5 EA showing how the DLL may be invoked.

Quick start

1) Run the TLS echo server (for local testing). Generate a self-signed cert first (openssl):

```powershell
openssl req -x509 -nodes -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -subj "/CN=localhost"
python echo_server/server.py --cert cert.pem --key key.pem
```

2) Run the core FastAPI (requires Python 3.11+):

```powershell
cd core
python -m pip install -r requirements.txt
python main.py
```

3) MQL5 sample EA: copy `mql5/sample_ea.mq5` into your MT5 Scripts/Experts folder and adjust DLL name/path.

Notes
- These are scaffolds for integration and testing. Implement production-grade features (mTLS, signing, FlatBuffers messages, batching, persistence) as described in the design docs.

FlatBuffers note
----------------
If you want binary FlatBuffers envelopes instead of JSON framing, install `flatc` and the FlatBuffers dev package. CMake will detect `flatc` and FlatBuffers and auto-generate `tripplex_generated.h` into the build folder. The DLL will then compile with `USE_FLATBUFFERS` enabled and send binary envelopes.
