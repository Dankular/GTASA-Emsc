# GTASA-Emsc

San Andreas browser-port workspace. Phase 1 targets a clean, standalone
Emscripten/WebAssembly build; Phase 2 targets the modern BrowserGameRT/WebGPU
specification.

The repository starts with the port contract and Emscripten probe only. The
original `gta-reversed` injected DLL is the upstream research source, not a
browser-ready dependency.

Start here:

- [`PORT_STATUS.md`](PORT_STATUS.md) — scope, blockers and acceptance gate
- [`emscripten/README.md`](emscripten/README.md) — first build probe
- [`emscripten/web/index.html`](emscripten/web/index.html) — browser smoke page

No proprietary game assets are included.

