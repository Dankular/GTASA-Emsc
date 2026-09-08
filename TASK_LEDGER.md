# Implementation task ledger

This ledger maps the Phase 1/2 specification gaps to repository work. Tasks
that require a detached, playable San Andreas engine remain explicitly blocked;
they are not represented as completed by the probe.

| Task | Spec gap | Status | Evidence |
|---|---|---|---|
| Toolchain pin + WASM module | P1.3/P2.0 | Done | `emscripten/CMakeLists.txt`, SDK 3.1.56 |
| Browser module boot/diagnostics | P1.3/P2.0 | Done | `emscripten/web/index.html` |
| Capability model/backend tier | P2.0 | Done | `runtime/include/browser_game_rt.h` |
| Device-loss controlled state | P2.0/P2.10 | Done | `RhiDevice::loseDevice` |
| RHI object contract | P2.1 | Scaffolded | buffer/texture/pipeline API follows next implementation slice |
| Render graph validation | P2.4 | Scaffolded | duplicate and read/write hazard checks |
| Priority stream scheduler | P1.4/P2.8 | Scaffolded | stable critical-first queue |
| Standalone SA entry point | P1.1 | Blocked | requires replacing injected `gta-reversed` hooks |
| Portable renderer/audio/input | P1.2 | Blocked | requires owned SA platform boundary |
| SA asset/save runtime | P1.4 | Blocked | requires playable standalone file contract and user fixtures |
| P1.6 acceptance route | P1.6 | Blocked | depends on P1.1–P1.5 |

The source inventory and native import evidence are now merged by
`tools/annotate-subsystems.ps1`. This closes the evidence-mapping gap; it does
not waive the standalone-SA gate. Each annotation carries its next detachment
task so implementation can proceed subsystem by subsystem.

The scaffold is intentionally additive: it does not copy proprietary game data
or reintroduce the injected DLL into this repository.
