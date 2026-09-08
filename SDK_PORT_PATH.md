# SDK-first SA port path

`gta-reversed` and `gta_reversed.asi` are no longer part of the Phase 1
bring-up path. The runtime now uses
[DK22Pac/plugin-sdk](https://github.com/DK22Pac/plugin-sdk), pinned under
`tools/plugin-sdk`, as the San Andreas class/layout/API reference.

This is a reference layer, not a proprietary asset bundle and not an injected
DLL. The SDK gives us the native contracts for vehicles, interiors, audio,
input, streaming, saves, camera and scripts. Browser implementations must own
their state and route platform services through `runtime/`.

Generate the SDK map with:

```powershell
.\tools\map-plugin-sdk.ps1
```

The next stage is adapter implementation plus behavior fixtures; no ASI loader
or `gta_sa.exe` process is required for the browser target.
