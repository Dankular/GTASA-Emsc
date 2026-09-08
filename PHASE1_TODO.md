# Phase 1 implementation TODO

This is the outstanding work required by section 68 of
`PLAY_III_WEB_STACK_CODEX.md`. A task is not complete until its acceptance
condition is demonstrated by the built runtime, not merely by a parser or unit
fixture.

## P1.0 — baseline and evidence

- [ ] Record exact compact-1.0 executable and installed-asset hashes in a versioned evidence manifest.
  - Acceptance: manifest reproduces the same hashes on a clean run.
- [ ] Add deterministic native boot checkpoints and crash logging.
  - Acceptance: a failed boot emits a timestamped checkpoint and stack/error record.
- [ ] Capture a repeatable native startup regression for the previously observed hang/crash.
  - Acceptance: clean native baseline starts without ASI injection.

## P1.1/P1.2 — owned engine and portable adapters

- [ ] Replace probe vehicle movement with the owned vehicle physics/state route.
  - Acceptance: vehicle movement, enter/exit, damage and save state survive the fixture route.
- [ ] Implement the camera subsystem and camera/input ABI.
  - Acceptance: camera follows the player and responds to keyboard/gamepad/touch input.
- [ ] Implement ped task execution beyond the deterministic fixture.
  - Acceptance: spawned peds navigate, collide and transition tasks during the route.
- [ ] Finish portable SDL3/platform boundaries.
  - Acceptance: no Win32/DirectInput/DirectSound dependency is required by runtime code.
- [ ] Add Linux/Clang CI for ABI and pointer-size assumptions.
  - Acceptance: native adapter and parser targets compile and run in CI.

## P1.3 — real browser boot

- [ ] Replace the probe page with a real `sa.wasm` engine entry point.
  - Acceptance: user gesture reaches a menu and starts a new game.
- [ ] Add WebGL2 renderer initialization and frame submission for decoded meshes.
  - Acceptance: a decoded SA mesh is visibly rendered in the game canvas.
- [ ] Surface module, graphics, audio and filesystem failures in the page UI.
  - Acceptance: forced failures are visible without opening devtools.

## P1.4 — assets, streaming and persistence

- [ ] Complete TXD decoding and upload of real texture pixels/materials.
  - Acceptance: a DFF material resolves to a decoded TXD texture in the renderer.
- [ ] Complete COL triangle/face collision, not only bounds and broadphase queries.
  - Acceptance: ray and movement collision use decoded collision faces.
- [ ] Implement IDE/IPL map loading and world placement.
  - Acceptance: a streamed sector places decoded geometry at its map transform.
- [ ] Integrate IMG/audio archives through range/chunk/OPFS-backed reads.
  - Acceptance: large archives are not eagerly copied into the WASM heap.
- [ ] Mount user-owned installs/packages through a browser import flow.
  - Acceptance: user-selected content is validated and mounted without repository assets.
- [ ] Complete save/settings persistence across page reload.
  - Acceptance: save, reload and load restore the gameplay route.

## P1.5 — browser parity

- [ ] Finish keyboard/mouse, gamepad and touch-as-gamepad gameplay mapping.
  - Acceptance: all three controls move the player in the same route.
- [ ] Implement effects, speech, mission audio and radio scheduling.
  - Acceptance: unlock/resume works after visibility changes and route audio plays.
- [ ] Implement fullscreen, resize, visibility pause and WebGL context-loss recovery.
  - Acceptance: each lifecycle transition preserves or safely restores runtime state.
- [ ] Add browser automation against the served page, not only static contract checks.
  - Acceptance: clean-profile browser run reaches menu, starts route and records result.
- [ ] Add MIME, cache, isolation and byte-range/chunk server tests.
  - Acceptance: hosted asset server passes all protocol checks.

## P1.6 — acceptance route

- [ ] Start a new game from the browser menu.
- [ ] Cross at least one world-streaming boundary.
- [ ] Complete a representative mission/script route with camera, ped, vehicle and interior behavior.
- [ ] Verify effects, speech, mission audio and radio.
- [ ] Save, reload the page, and load the save.
- [ ] Verify no native executable, DLL injection, original address or proprietary repository asset is required.
- [ ] Produce a clean-profile acceptance capture and tag the resulting `sa.wasm` build.

Phase 1 is complete only when every checkbox above is checked with linked build
evidence. Current parser/runtime fixtures do not satisfy the final acceptance
route by themselves.
