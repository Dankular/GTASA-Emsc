$ErrorActionPreference = 'Stop'

$root = Resolve-Path (Join-Path $PSScriptRoot '..')
$build = Join-Path $root 'build'
$web = Join-Path $root 'web'

if (-not (Get-Command emcmake -ErrorAction SilentlyContinue)) {
    throw 'emcmake was not found. Activate the Emscripten SDK before running this script.'
}

New-Item -ItemType Directory -Force -Path $build, $web | Out-Null
emcmake cmake -S $root -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build $build --target sa_port_probe

$js = Join-Path $build 'sa_port_probe.js'
if (-not (Test-Path -LiteralPath $js)) {
    throw "Expected Emscripten output was not created: $js"
}

Copy-Item -LiteralPath $js -Destination (Join-Path $web 'sa_port_probe.js') -Force
Copy-Item -LiteralPath (Join-Path $build 'sa_port_probe.wasm') -Destination (Join-Path $web 'sa_port_probe.wasm') -Force
Write-Host "WASM probe written to $web"

