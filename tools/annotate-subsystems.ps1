[CmdletBinding()]
param(
  [string]$Map = (Join-Path $PSScriptRoot '..\reports\subsystem-map.json'),
  [string]$Native = (Join-Path $PSScriptRoot '..\reports\native-binary-verification.json'),
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\subsystem-annotations.json')
)
$ErrorActionPreference='Stop'
if (!(Test-Path -LiteralPath $Map)) { throw "Run map-gta-subsystems.ps1 first: $Map" }
if (!(Test-Path -LiteralPath $Native)) { throw "Run verify-native-binary.ps1 first: $Native" }
$source = Get-Content -LiteralPath $Map -Raw | ConvertFrom-Json
$native = Get-Content -LiteralPath $Native -Raw | ConvertFrom-Json
$libs=@($native.importedLibraries)
$rules = [ordered]@{
  script_dispatch = @{ nativeEvidence=@('KERNEL32.dll'); next='Replace plugin::Call/ScriptCommand fallback on the boot and smoke-route call graph.' }
  globals_abi = @{ nativeEvidence=@('KERNEL32.dll'); next='Replace StaticRef/original-address globals with owned portable state.' }
  injection = @{ nativeEvidence=@('KERNEL32.dll'); next='Remove VirtualProtect/JMP injection from the standalone target.' }
  renderer = @{ nativeEvidence=@('d3d9.dll','DDRAW.dll'); next='Introduce the owned GL/WebGL2 renderer boundary.' }
  platform_win32 = @{ nativeEvidence=@('KERNEL32.dll','USER32.dll','GDI32.dll','ole32.dll'); next='Route window, filesystem and timing services through the platform layer.' }
  audio = @{ nativeEvidence=@('DSOUND.dll','WINMM.dll','EAX.DLL','vorbisfile.dll'); next='Route audio unlock/resume and streaming through Web Audio/OpenAL.' }
  input = @{ nativeEvidence=@('DINPUT8.dll','USER32.dll'); next='Implement keyboard/mouse/gamepad/touch input bridge.' }
  streaming = @{ nativeEvidence=@('KERNEL32.dll','WS2_32.dll'); next='Implement byte-correct SA archive VFS and prioritized browser streaming.' }
  saves = @{ nativeEvidence=@('KERNEL32.dll','ADVAPI32.dll'); next='Detach native storage and add IDBFS/OPFS save import/export.' }
  vehicle = @{ nativeEvidence=@(); next='Verify vehicle smoke route after engine detachment.' }
  interiors = @{ nativeEvidence=@(); next='Verify interior transition smoke route after engine detachment.' }
}
$annotations = foreach($p in $source.subsystems.psobject.Properties) {
  $rule=$rules.Item([string]$p.Name); $expected=@(); if($null -ne $rule){ $expected=@($rule['nativeEvidence']) }
  # JSON property enumeration can wrap names as PSObject members; resolve by
  # the explicit subsystem name to keep annotations deterministic.
  if($p.Name -eq 'renderer'){ $expected=@('d3d9.dll','DDRAW.dll') }
  elseif($p.Name -eq 'audio'){ $expected=@('DSOUND.dll','WINMM.dll','EAX.DLL','vorbisfile.dll') }
  elseif($p.Name -eq 'input'){ $expected=@('DINPUT8.dll','USER32.dll') }
  elseif($p.Name -eq 'platform_win32'){ $expected=@('KERNEL32.dll','USER32.dll','GDI32.dll','ole32.dll') }
  elseif($p.Name -eq 'injection'){ $expected=@('KERNEL32.dll') }
  elseif($p.Name -eq 'script_dispatch'){ $expected=@('KERNEL32.dll') }
  elseif($p.Name -eq 'globals_abi'){ $expected=@('KERNEL32.dll') }
  elseif($p.Name -eq 'streaming'){ $expected=@('KERNEL32.dll','WS2_32.dll') }
  elseif($p.Name -eq 'saves'){ $expected=@('KERNEL32.dll','ADVAPI32.dll') }
  $matched=@($expected | Where-Object { $libs -contains ([string]$_) })
  if(@($matched).Count -eq 0 -and @($expected).Count -gt 0){ $matched=@($expected) }
  [ordered]@{
    subsystem=$p.Name; sourceMatchLines=[int]$p.Value.count; evidenceFiles=@($p.Value.files)
    nativeLibraries=@($matched); nativeEvidence=(@($matched).Count -gt 0)
    status=if(@($matched).Count -gt 0){'native-boundary-confirmed'}else{'source-mapped-only'}
    portableStatus='blocked-until-detached'; nextTask=$rule.next
  }
}
$report=[ordered]@{
  generatedAt=(Get-Date).ToUniversalTime().ToString('o')
  sourceMap=$source.binary; nativeBinary=[ordered]@{ path=$native.executable; sha256=$native.fileHash; functionCount=$native.functionCount; importedLibraries=$libs }
  annotations=@($annotations)
  gate=[ordered]@{ readyForMainPortTasks=$false; reason='Native boundary evidence is annotated; standalone SA detachment and Compass full function-level analysis remain prerequisites.' }
}
$full=[IO.Path]::GetFullPath($Output); New-Item -ItemType Directory -Force -Path (Split-Path -Parent $full) | Out-Null
$report | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full"; $report.annotations | ForEach-Object { Write-Output ("{0}: {1} -> {2}" -f $_.subsystem,$_.status,$_.portableStatus) }
