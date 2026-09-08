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

No proprietary game assets are included.
