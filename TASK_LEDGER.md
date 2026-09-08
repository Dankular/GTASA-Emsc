# Implementation task ledger

This ledger maps the Phase 1/2 specification gaps to repository work. The
active route is SDK-first: `plugin-sdk/game_sa` is the native reference and the
browser owns implementation state. `gta-reversed`/ASI work is not a gate.

| Task | Spec gap | Status | Evidence |
|---|---|---|---|
| Toolchain pin + WASM module | P1.3/P2.0 | Done | `emscripten/CMakeLists.txt`, SDK 3.1.56 |
| Browser module boot/diagnostics | P1.3/P2.0 | Done | `emscripten/web/index.html` |
| Capability model/backend tier | P2.0 | Done | `runtime/include/browser_game_rt.h` |
| Device-loss controlled state | P2.0/P2.10 | Done | `RhiDevice::loseDevice` |
| RHI object contract | P2.1 | Scaffolded | buffer/texture/pipeline API follows next implementation slice |
| Render graph validation | P2.4 | Scaffolded | duplicate and read/write hazard checks |
| Priority stream scheduler | P1.4/P2.8 | Scaffolded | stable critical-first queue |
| SDK class/API inventory | P1.1 | Done | `tools/map-plugin-sdk.ps1`, pinned SDK commit |
| Portable subsystem adapters | P1.2 | In progress | `runtime/sa_subsystem_bridge.h`, all eleven tracks registered |
| SA asset/save runtime | P1.4 | Next | SDK contracts plus user-owned fixture/VFS adapters |
| Browser feature parity | P1.5 | Next | input/audio/touch/save/stream tests |
| P1.6 acceptance route | P1.6 | Pending | begins after adapters and legal fixture pass |

The source inventory and native import evidence are now merged by
`tools/annotate-subsystems.ps1`. This closes the evidence-mapping gap; it does
not waive the standalone-SA gate. Each annotation carries its next detachment
task so implementation can proceed subsystem by subsystem.

All eleven tracks now enter the compiled `SaSubsystemBridge` together. The
SDK map supplies the vehicle/interior contracts that were previously tied to
the ASI path; both are now adapter tasks rather than native-injection gates.

The scaffold is intentionally additive: it does not copy proprietary game data
or reintroduce the injected DLL into this repository.
