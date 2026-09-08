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
| Portable subsystem adapters | P1.2 | In progress | `runtime/sa_subsystem_bridge.h` + `runtime/sa_adapters.h`, all eleven tracks registered; owned gameplay state crosses WASM ABI |
| SA asset/save runtime | P1.4 | In progress | `runtime/sa_services.h`: VFS ranges, saves import/export |
| Browser feature parity | P1.5 | In progress | `browser_services.js`: input/audio/lifecycle/IndexedDB/RangeVfs plus canvas gameplay loop |
| P1.6 acceptance route | P1.6 | In progress | browser page now runs gameplay tick, RangeVfs fixture read and IndexedDB save round-trip; full engine route remains source-dependent |
| Binary evidence walker | P1.1/P1.2 | Done | `tools/walk-sa-binary.ps1`; PE fingerprint, imports, sections, symbols and bounded subsystem string evidence |
| Three-call validation | P1.3/P1.4/P1.6 | Done for runtime probe | `tools/run-next-three.ps1`; native boot/services and Emscripten WASM assembly all pass |
| Installed-content boundary | P1.4 | Implemented | `tools/index-sa-assets.ps1`; hashes local user-owned content without committing assets |

The source inventory and native import evidence are now merged by
`tools/annotate-subsystems.ps1`. This closes the evidence-mapping gap; it does
not waive the standalone-SA gate. Each annotation carries its next detachment
task so implementation can proceed subsystem by subsystem.

All eleven tracks now enter the compiled `SaSubsystemBridge` together. The
SDK map supplies the vehicle/interior contracts that were previously tied to
the ASI path; both are now adapter tasks rather than native-injection gates.

The scaffold is intentionally additive: it does not copy proprietary game data
or reintroduce the injected DLL into this repository.

The SDK adapter baseline now has a deterministic smoke route covering script
dispatch, vehicle movement, interior entry/exit, save and reload. Native and
WASM builds both compile this route; it is a contract test, not yet the full
commercial game's gameplay route.

The four service tracks are now implemented together: `AssetVfs` (normalized
random-access reads), `LogicalInput` (keyboard/gamepad/touch state),
`BrowserAudio` (gesture unlock and queued playback), and `PersistentSaves`
(slot read/write plus import/export serialization). `sa_services_test` passes
alongside the WASM build.

The browser parity shell now wires keyboard/gamepad input, audio unlock/resume,
fullscreen, visibility pause/resume, IndexedDB save testing and Range-backed
asset reads into the page. The remaining P1.5 work is gameplay integration and
browser automation against the eventual `sa.wasm`, not another platform API
design pass.

The owned executable is now a first-class behavioral reference through
`tools/walk-sa-binary.ps1`. Its fingerprinted imports, sections and discovered
functions feed the same subsystem adapter map; implementation proceeds by
replacing classified boundaries in `runtime/`, never by linking or executing
the original binary in WebAssembly. The walker is evidence only; it cannot
recover missing engine source or produce a playable GTA binary by itself.

The WASM boundary is now integrated through `GameRuntimeController`: browser
input is sampled every animation frame and passed to runtime ticks, runtime
initialization is explicit, and vehicle/interior entry calls cross the same
exported ABI. The current probe exercises that contract; the production engine
will replace the probe exports without changing the browser controller.
