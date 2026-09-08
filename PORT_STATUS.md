# GTA San Andreas Emscripten port

This repository is the Phase 1 San Andreas port target. The source baseline is
`gta-reversed`; the current `main` commit is an imported baseline only. The
injected ASI/DLL build is **not** a browser build and must not be treated as
Phase 1 complete.

## First milestone: toolchain and browser contract

The `emscripten/` directory contains a deliberately small probe target. It
proves that this repository can produce an ES-module WebAssembly artifact and
launch it from a canvas without pulling the original executable into the
browser. It is a port-health check, not a playable game.

```powershell
emscripten\scripts\build-web.ps1
python -m http.server 8080 --directory emscripten\web
```

Open `http://localhost:8080/` and verify that the page reports the compiled
module and the `__EMSCRIPTEN__` build flag.

## Phase 1 blockers

Before the real SA game can be linked, the source must be detached from:

- original-executable `plugin::Call*` fallbacks;
- absolute-address `StaticRef` globals and vtables;
- `VirtualProtect`/JMP injection and the ASI entry point;
- hard-coded RenderWare/D3D9 calls;
- Win32/DirectSound/DirectInput-only platform paths;
- x86 calling-convention and pointer-size assumptions.

The acceptance gate is defined in
`PLAY_III_WEB_STACK_CODEX.md`, section 68 (`P1.6`). Phase 2 does not begin
until that gate has a tagged `sa.wasm` build with browser input, audio, saves,
asset loading and a representative gameplay smoke route.

