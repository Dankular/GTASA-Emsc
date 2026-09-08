[CmdletBinding()]
param(
  [string]$GameRoot = 'C:\Program Files (x86)\Rockstar Games\GTA San Andreas',
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\sa-asset-manifest.json')
)
$ErrorActionPreference = 'Stop'
if (!(Test-Path -LiteralPath $GameRoot)) { throw "Missing game root: $GameRoot" }
$root = (Resolve-Path $GameRoot).Path
$files = Get-ChildItem -LiteralPath $root -File -Recurse | Where-Object { $_.Extension -in @('.img','.txd','.dff','.col','.ifp','.ipl','.ide','.dat','.scm','.wav','.ogg','.mp3') }
$entries = @($files | ForEach-Object {
  $relative = $_.FullName.Substring($root.Length).TrimStart('\','/') -replace '\\','/'
  [ordered]@{ path=$relative; size=$_.Length; sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }
})
$manifest = [ordered]@{ generatedAt=(Get-Date).ToUniversalTime().ToString('o'); gameRoot=$root; fileCount=$entries.Count; files=$entries; policy='Manifest only; proprietary assets remain outside this repository and are mounted by the browser VFS.' }
$full=[IO.Path]::GetFullPath($Output); New-Item -ItemType Directory -Force -Path (Split-Path $full) | Out-Null
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full ($($entries.Count) files)"
