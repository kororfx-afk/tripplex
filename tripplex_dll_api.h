#pragma once

#ifdef _WIN32
#  ifdef TPX_EXPORTS
#    define TPX_API __declspec(dllexport)
#  else
#    define TPX_API __declspec(dllimport)
#  endif
#else
#  define TPX_API
#endif

#include <stdint.h>

// Minimal C ABI for MQL5 consumption. Use JSON for control payloads and
// FlatBuffers binary for market/order payloads in high-performance paths.
extern "C" {

// Initialize the DLL. `config_json` contains endpoint, cert paths, timeouts.
TPX_API int TPX_Init(const char* config_json);

// Authenticate using token or trigger mTLS handshake validation.
TPX_API int TPX_Auth(const char* auth_token);

// Subscribe/unsubscribe to symbol L1/L2.
TPX_API int TPX_Subscribe(const char* symbol, int depth);
TPX_API int TPX_Unsubscribe(const char* symbol);

// Send an order. `order_payload` may be JSON or base64 FlatBuffer. Return 0 on accepted.
TPX_API int TPX_SendOrder(const char* order_payload, const char* client_order_id);
TPX_API int TPX_ModifyOrder(const char* order_id, const char* update_payload);
TPX_API int TPX_CancelOrder(const char* order_id);

// Synchronous query helper. Caller provides `out_buf` and `out_buf_len`.
TPX_API int TPX_Query(const char* query_json, char* out_buf, int out_buf_len);

// Heartbeat sequence number
TPX_API int TPX_Heartbeat(uint64_t seq);

// Graceful shutdown
TPX_API void TPX_Shutdown();

}
