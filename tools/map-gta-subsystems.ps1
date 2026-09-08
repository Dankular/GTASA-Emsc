[CmdletBinding()]
param(
  [string]$GtaRoot = (Join-Path $PSScriptRoot '..\..\gta-reversed'),
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\subsystem-map.json'),
  [string]$Compass = (Join-Path $PSScriptRoot 'binary\build\src\cli\compass-cli.exe')
)
$ErrorActionPreference = 'Stop'
$GtaRoot = (Resolve-Path -LiteralPath $GtaRoot).Path
$sourceRoot = Join-Path $GtaRoot 'source'
if (!(Test-Path -LiteralPath $sourceRoot)) { throw "Missing gta-reversed source directory: $sourceRoot" }
function Count-Matches([string]$Pattern, [string[]]$Files) {
  $hits = @(Select-String -Path $Files -Pattern $Pattern -AllMatches)
  [pscustomobject]@{ count=$hits.Count; files=@($hits | ForEach-Object { $_.Path.Replace($GtaRoot + '\','') } | Sort-Object -Unique | Select-Object -First 40) }
}
$files = @(Get-ChildItem -LiteralPath $sourceRoot -Recurse -File -Include *.h,*.hpp,*.c,*.cc,*.cpp | ForEach-Object FullName)
$hooks = Join-Path $GtaRoot 'docs\hooks.csv'
$hookRows = if (Test-Path -LiteralPath $hooks) { @(Import-Csv -LiteralPath $hooks) } else { @() }
$binaryCommit = if (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'binary\.git')) { git -C (Join-Path $PSScriptRoot 'binary') rev-parse HEAD } else { 'unavailable' }
$map = [ordered]@{
  generatedAt=(Get-Date).ToUniversalTime().ToString('o'); gtaRoot=$GtaRoot; sourceFiles=$files.Count
  binary=[ordered]@{ repository='https://github.com/Dankular/Binary'; commit=$binaryCommit.Trim(); cliPresent=(Test-Path -LiteralPath $Compass) }
  hookRows=$hookRows.Count
  subsystems=[ordered]@{
    script_dispatch=Count-Matches 'plugin::Call|ScriptCommand|Command<|ProcessOneCommand' $files
    globals_abi=Count-Matches 'StaticRef|StaticAddress|CVehicle::|CPed::' $files
    injection=Count-Matches 'VirtualProtect|InjectHook|RedirectCall|ReversibleHook' $files
    renderer=Count-Matches 'RenderWare|Rw[A-Z]|D3D9|Direct3D|RwEngine' $files
    platform_win32=Count-Matches 'Windows\.h|Win32|CreateFile|GetAsyncKeyState|HWND|DirectInput' $files
    audio=Count-Matches 'DirectSound|Miles|Audio|AE[A-Z]|SampleManager' $files
    input=Count-Matches 'Pad|Gamepad|Mouse|Keyboard|DI[A-Z]' $files
    streaming=Count-Matches 'CStreaming|IMG|CdStream|RwStream|Stream' $files
    saves=Count-Matches 'CGenericGameStorage|Save|Load|\.b' $files
    vehicle=Count-Matches 'CVehicle|CAutomobile|CBoat|CTrain' $files
    interiors=Count-Matches 'Interior|InteriorManager|ThePaths|CEntryExit' $files
  }
  interpretation=[ordered]@{ verified='Binary commit is pinned; source and hook inventories are reproducible.'; next='Run Compass analysis on a legal native fixture or executable, then attach function-level annotations to this subsystem map.'; rule='Counts are evidence for porting work, not proof that a subsystem is browser-ready.' }
}
$full=[IO.Path]::GetFullPath($Output); New-Item -ItemType Directory -Force -Path (Split-Path -Parent $full) | Out-Null
$map | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full"; Write-Output "Scanned $($files.Count) source files and $($hookRows.Count) hook rows"
