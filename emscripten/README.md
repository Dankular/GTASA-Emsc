# Emscripten bootstrap

This is the first executable port seam. It intentionally builds only
`sa_port_probe`; the main `gta_reversed` target still contains injection and
original-address dependencies and is not linked here.

## Build

Activate an Emscripten SDK, then run from PowerShell:

```powershell
.\scripts\build-web.ps1
python -m http.server 8080 --directory .\web
```

The output is an ES-module loader plus `sa_port_probe.wasm`. The next port
change should replace the probe with a portable platform entry point only after
the dependency ledger identifies the first detached SA subsystem.

