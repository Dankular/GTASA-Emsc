# Reverse-engineering mapping tools

`binary/` is a pinned submodule of [Dankular/Binary](https://github.com/Dankular/Binary),
currently at commit `0c3478452a8f607b04b6105b1a00037f3730f647`. It is an analysis
tool, not a browser-runtime dependency, and no GTA assets are copied here.

Run the deterministic GTA inventory:

```powershell
.\tools\map-gta-subsystems.ps1
```

The default sibling path is `..\gta-reversed`; use `-GtaRoot` to override it.
The report is written to `reports/subsystem-map.json` and records source/hook
counts plus evidence files for script dispatch, globals/ABI, injection,
rendering, platform, audio, input, streaming, saves, vehicles and interiors.

For function-level verification, build Compass separately and pass its
`compass-cli` path with `-Compass`; analyze only a legal local executable or
fixture and keep proprietary binary data out of this repository.

Walk the owned SA executable directly with the Binary-compatible backend:

```powershell
.\tools\walk-sa-binary.ps1
```

This creates a fingerprinted function/import/section map, bounded string evidence,
and classifies native boundaries for renderer, audio, input, platform, streaming
and networking. The map is implementation evidence; it never copies executable
bytes or permits original-address calls in the WASM runtime. The default target
is the installed `gta_sa.exe` (override with `-Executable` for a legal fixture).

Run the three acceptance calls (native boot, services/VFS, and WASM assembly):

```powershell
.\tools\run-next-three.ps1
```

The result is written to `reports/next-three.json`; generated reports are ignored.

Capture the P1.0 baseline without copying game bytes into the repository:

```powershell
.\tools\capture-native-baseline.ps1
```

The report contains SHA-256 evidence for the compact executable, startup
executable, and supported installed asset files. It also records ordered boot
checkpoints, timeout/exit status, and separate stdout/stderr logs. The content
digest is based only on sorted relative paths, sizes, and hashes, so rerunning
against an unchanged install reproduces the same identity. `-SkipStartup` is
available when only evidence hashing is required; use an explicit `-Output` in
CI to retain the capture artifact.

Run the fixture regression for the capture tool itself:

```powershell
.\tools\test-baseline-evidence.ps1
```

The runner also performs the browser contract gate (required web files, WASM
header, generated ABI names, browser service APIs, and native-execution
boundary):

```powershell
node .\tools\browser-contract-smoke.mjs
```

Serve the browser artifact with the same cross-origin isolation, MIME, cache,
and byte-range behavior expected by the runtime:

```powershell
node .\tools\serve-browser.mjs 8080
node .\tools\browser-protocol-smoke.mjs
```

The protocol gate validates menu/new-game markup, WASM MIME, `206` range
chunks, invalid-range `416`, cache policy, COOP/COEP isolation, and visible
`404` failures. It is safe to run in CI without a proprietary game install.

Index a local, user-owned San Andreas installation without copying any assets:

```powershell
.\tools\index-sa-assets.ps1
```

The manifest records relative paths, sizes and hashes only. It is the input
contract for mounting real local content through `RangeVfs`.

On Windows, the companion verification script can use the downloaded Rizin
backend while Compass's POSIX-only sandbox sources are unavailable:

```powershell
.\tools\verify-native-binary.ps1
```

Merge both evidence sets into subsystem annotations:

```powershell
.\tools\annotate-subsystems.ps1
```
