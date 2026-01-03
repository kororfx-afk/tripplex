Tripplex DLL prototype
=====================

This folder contains a minimal C++ DLL prototype (`tripplex`) with a small exported C ABI. It is a prototype for local testing and intentionally omits TLS and FlatBuffers serialization — both should be added for production.

Build (Windows, with CMake & Visual Studio):

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The resulting DLL will be `tripplex.dll` in the build output.

Usage
- Place `tripplex.dll` into MT5 `MQL5\Libraries` or load it from your EA.
- Configure endpoint via `TPX_Init("127.0.0.1:9000")` or JSON containing `endpoint`.

Next steps
- Add OpenSSL/mbedTLS for mTLS connections.
- Optional: build with OpenSSL to enable TLS support. CMake will detect OpenSSL and link automatically.
- Example (with OpenSSL available):

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

When OpenSSL is found, the DLL will attempt TLS connections to the configured endpoint.
- Replace naive payloads with FlatBuffers `Envelope` payloads.
- Implement async connection pooling and batching.
