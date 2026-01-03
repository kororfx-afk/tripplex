# Tripplex DLL API (draft)

Overview
--------
This document defines the minimal, high-performance DLL API surface used by MQL5 adapters and the Tripplex Core. Transport is TLS-framed messages (prefer FlatBuffers for low-latency payloads). All messages are versioned and carry a correlation id (UUID).

Design goals
- Lightweight exported C ABI for easy linking from MQL5.
- Idempotent order submission via client-generated UUIDs.
- Batched/coalesced market data push from DLL to Core.
- Mutual TLS for authentication; token exchange on Init.

API Surface (C exports)
- `TPX_Init(const char *config_json)` — Initialize DLL, configure Core endpoint, TLS cert paths, timeouts.
- `TPX_Auth(const char *auth_token)` — Perform auth handshake (client token or mTLS handshake verification).
- `TPX_Subscribe(const char *symbol, int depth)` — Subscribe symbol L1/L2.
- `TPX_Unsubscribe(const char *symbol)` — Remove subscription.
- `TPX_SendOrder(const char *order_payload_json, const char *client_order_id)` — Submit order; returns immediate status code.
- `TPX_ModifyOrder(const char *order_id, const char *update_payload_json)` — Modify order.
- `TPX_CancelOrder(const char *order_id)` — Cancel order.
- `TPX_Query(const char *query_json, char *out_buf, int out_buf_len)` — Synchronous query helper for positions/account.
- `TPX_Heartbeat(uint64_t seq)` — Heartbeat ping to Tripplex Core.
- `TPX_Shutdown()` — Graceful shutdown.

Serialization & framing
- Use FlatBuffers schema in `/schemas/tripplex.fbs` for main messages. JSON may be used for control/debug.
- Messages framed with length-prefixed header; optional sequence number and CRC.

Batching
- Implement coalescing windows (configurable, default 10ms) for ticks and acks.

Idempotency
- Clients must supply `client_order_id` (UUID). Core persists processed keys to prevent duplicate executions.

Error handling
- Functions return integer error codes for in-call failures; async failures are delivered as events to Core.

Security
- Sign DLL binary and validate on load.
- Use mTLS with certificate pinning; rotate certs via admin dashboard.

Versioning
- Messages and API surface must include a `version` field. Maintain backward-compatible additions; breaking changes require major version bump.

Next steps
- Iterate FlatBuffers schema and generate bindings for C++ and Python.
- Provide a minimal C++ header with exported symbols (added in repo).
