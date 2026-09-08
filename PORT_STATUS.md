# GTA San Andreas Emscripten port

This repository is the Phase 1 San Andreas port target. The native reference
layer is `DK22Pac/plugin-sdk` under `tools/plugin-sdk`; `gta-reversed` and its
ASI are research history only and are not required by the browser build.

## First milestone: toolchain and browser contract

The `emscripten/` directory contains a deliberately small probe target. It
proves that this repository can produce an ES-module WebAssembly artifact and
launch it from a canvas without pulling the original executable into the
browser. It is a port-health check, not a playable game.

The additive `runtime/` scaffold covers the first actionable Phase 2 gaps:
capability/tier reporting, controlled device-loss state, render-graph hazard
validation and a priority stream scheduler. See [`TASK_LEDGER.md`](TASK_LEDGER.md)
for the dependency map and source-dependent blockers.

```powershell
emscripten\scripts\build-web.ps1
python -m http.server 8080 --directory emscripten\web
```

Open `http://localhost:8080/` and verify that the page reports the compiled
module and the `__EMSCRIPTEN__` build flag.

## SDK-first next stage

The next implementation stage uses the pinned SDK reference and the compiled
`SaSubsystemBridge`. Run the SDK inventory before adapter work:

```powershell
.\tools\map-plugin-sdk.ps1
```

The browser target owns its state and platform services. The SDK supplies
native class contracts and behavior references; it is not loaded as an ASI.

## Remaining gaps

The remaining browser gaps are adapter implementations, not an ASI detachment:

- SDK-backed script dispatch and owned globals;
- portable replacement for absolute-address state and vtables;
- browser-owned startup and lifecycle;
- hard-coded RenderWare/D3D9 calls;
- Win32/DirectSound/DirectInput-only platform paths;
- x86 calling-convention and pointer-size assumptions.

The acceptance gate is defined in
`PLAY_III_WEB_STACK_CODEX.md`, section 68 (`P1.6`). Phase 2 does not begin
until that gate has a tagged `sa.wasm` build with browser input, audio, saves,
asset loading and a representative gameplay smoke route.
