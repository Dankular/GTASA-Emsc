# GTASA-Emsc

San Andreas browser-port workspace. Phase 1 targets a clean, standalone
Emscripten/WebAssembly build; Phase 2 targets the modern BrowserGameRT/WebGPU
specification.

The repository now uses `DK22Pac/plugin-sdk` as the native San Andreas
class/layout/API reference. `gta-reversed` and its injected ASI are historical
research inputs only; they are not build or runtime dependencies.

Start here:

- [`PORT_STATUS.md`](PORT_STATUS.md) — scope, blockers and acceptance gate
- [`emscripten/README.md`](emscripten/README.md) — first build probe
- [`emscripten/web/index.html`](emscripten/web/index.html) — browser smoke page
- [`SDK_PORT_PATH.md`](SDK_PORT_PATH.md) — SDK-first migration contract
- [`BINARY_MAPPING.md`](BINARY_MAPPING.md) — reverse-engineering evidence contract
- [`TASK_LEDGER.md`](TASK_LEDGER.md) — implementation gaps and acceptance state
- `runtime/tests/sa_adapter_test.cpp` — native SDK-adapter smoke route
- `runtime/tests/sa_services_test.cpp` — VFS/input/audio/save smoke route
- `emscripten/web/browser_services.js` — browser input/audio/save/range services
- `GameRuntimeController` — frame loop and WASM ABI integration

No proprietary game assets are included.

Build and run the adapter baseline:

```powershell
emscripten\scripts\build-web.ps1
cmake -S emscripten -B emscripten\build-native -G "Visual Studio 17 2022" -A x64
cmake --build emscripten\build-native --config Release --target sa_adapter_test
emscripten\build-native\Release\sa_adapter_test.exe
cmake --build emscripten\build-native --config Release --target sa_services_test
emscripten\build-native\Release\sa_services_test.exe
cmake --build emscripten\build-native --config Release --target sa_img_archive_test
emscripten\build-native\Release\sa_img_archive_test.exe
```
