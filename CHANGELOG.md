# Changelog

All notable changes to this project will be documented in this file.

## [0.1.0] - 2026-01-07
- Initial prototype: FastAPI backend + Expo frontend
- Image upload, thumbnail generation, and analyzer (trend, peaks/troughs, support/resistance)
- Trade signal generation (entry, stop_loss, take_profit, breakeven, confidence, risk guidance)
- Auth: signup/login/logout with refresh tokens and token revocation
- Optional OpenAI integration (feature-flagged via `ENABLE_OPENAI` and `OPENAI_API_KEY`)
- Dockerfile, `docker-compose.yml`, and GitHub Actions CI for E2E tests
- EAS/EAS config scaffold for mobile builds
- Docs: `MOBILE_BUILD.md`, `CI_NOTES.md`, `PROD_HARDENING.md`

