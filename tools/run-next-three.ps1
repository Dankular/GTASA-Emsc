[CmdletBinding()]
param(
  [string]$Emsdk = 'D:\Dev Proj\GTA Uno\GTASA\emsdk',
  [string]$Report = (Join-Path $PSScriptRoot '..\reports\next-three.json')
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$native = Join-Path $root 'emscripten\build-native'
$results = [ordered]@{}

function Invoke-Step([string]$name, [scriptblock]$body) {
  $sw = [Diagnostics.Stopwatch]::StartNew()
  try { & $body; $results[$name] = [ordered]@{ passed = $true; exitCode = 0; elapsedMs = $sw.ElapsedMilliseconds } }
  catch { $results[$name] = [ordered]@{ passed = $false; exitCode = 1; elapsedMs = $sw.ElapsedMilliseconds; error = $_.Exception.Message } }
}

Invoke-Step 'bootSmoke' {
  if (!(Test-Path (Join-Path $native 'CMakeCache.txt'))) { cmake -S (Join-Path $root 'emscripten') -B $native -G 'Visual Studio 17 2022' -A x64 }
  cmake --build $native --config Release --target sa_adapter_test
  $exe = Join-Path $native 'Release\sa_adapter_test.exe'
  & $exe
  if ($LASTEXITCODE -ne 0) { throw "sa_adapter_test failed with exit code $LASTEXITCODE" }
}

Invoke-Step 'servicesSmoke' {
  cmake --build $native --config Release --target sa_services_test
  $exe = Join-Path $native 'Release\sa_services_test.exe'
  & $exe
  if ($LASTEXITCODE -ne 0) { throw "sa_services_test failed with exit code $LASTEXITCODE" }
}

Invoke-Step 'wasmAssembly' {
  if (Test-Path (Join-Path $Emsdk 'emsdk_env.ps1')) { & (Join-Path $Emsdk 'emsdk_env.ps1') | Out-Null }
  & (Join-Path $root 'emscripten\scripts\build-web.ps1')
  $wasm = Join-Path $root 'emscripten\web\sa_port_probe.wasm'
  $js = Join-Path $root 'emscripten\web\sa_port_probe.js'
  if (!(Test-Path $wasm) -or !(Test-Path $js)) { throw 'WASM assembly outputs are missing' }
  $magic = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($wasm)[0..3])
  if ($magic -ne "`0asm") { throw "Invalid WASM magic: $magic" }
}

$full = [IO.Path]::GetFullPath($Report)
New-Item -ItemType Directory -Force -Path (Split-Path $full) | Out-Null
$results | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full"
if (@($results.Values | Where-Object { -not $_.passed }).Count -gt 0) { exit 1 }
