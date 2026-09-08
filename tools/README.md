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

On Windows, the companion verification script can use the downloaded Rizin
backend while Compass's POSIX-only sandbox sources are unavailable:

```powershell
.\tools\verify-native-binary.ps1
```

Merge both evidence sets into subsystem annotations:

```powershell
.\tools\annotate-subsystems.ps1
```
