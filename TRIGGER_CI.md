Trigger CI

Push any commit to `main` (or run the workflow manually) to trigger the `test-backend` CI workflow.

Quick command to create a tiny commit and push (run from repo root):

```bash
git add TRIGGER_CI.md
git commit -m "ci: trigger test-backend"
git push origin main
```

Manual run:
- Go to the repository Actions tab → select the `test-backend` workflow → "Run workflow" → choose branch `main` → Run.

Notes:
- Ensure you added the repository secrets `OPENAI_API_KEY`, `ENABLE_OPENAI` (value `1`) and `ADMIN_TOKEN` before running, otherwise model-backed tests will be skipped or fail.
- After the run, open the workflow run and inspect the "Run E2E tests" step logs for `tests/e2e_test.py` output; look for `/analyze` responses and any `model_analysis` or `model_analysis_error` fields in the JSON.
