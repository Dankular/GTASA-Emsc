[CmdletBinding()]
param(
  [string]$Executable = 'C:\Program Files (x86)\Rockstar Games\GTA San Andreas\gta_sa_compact1.0.exe',
  [string]$RizinBin = 'D:\Dev Proj\GTA Uno\GTASA\rizin-static\rizin-win-installer-vs2019_static-64\bin\rz-bin.exe',
  [string]$Rizin = 'D:\Dev Proj\GTA Uno\GTASA\rizin-static\rizin-win-installer-vs2019_static-64\bin\rizin.exe',
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\native-binary-verification.json')
)
$ErrorActionPreference='Stop'
if (!(Test-Path -LiteralPath $Executable)) { throw "Executable not found: $Executable" }
if (!(Test-Path -LiteralPath $RizinBin)) { throw "rz-bin not found: $RizinBin" }
if (!(Test-Path -LiteralPath $Rizin)) { throw "rizin not found: $Rizin" }
$infoText = (& $RizinBin -Ij $Executable | Out-String)
$importsText = (& $RizinBin -ij $Executable 2>$null)
$functionsText = (& $Rizin -2 -q -c 'aa; aflj;q' $Executable | Out-String)
$info = try { $infoText | ConvertFrom-Json } catch { @{} }
$functions = try { $functionsText | ConvertFrom-Json } catch { @() }
$imports = try { $importsText | ConvertFrom-Json } catch { @() }
$report=[ordered]@{
  generatedAt=(Get-Date).ToUniversalTime().ToString('o'); executable=(Resolve-Path $Executable).Path
  fileHash=(Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
  analyzer=[ordered]@{ repository='https://github.com/Dankular/Binary'; backend='Rizin-compatible analysis path'; toolVersion=((& $RizinBin -v) | Select-Object -First 1) }
  binary=$info.info
  functionCount=@($functions).Count
  functions=@($functions | Select-Object name,offset,size,nbbs,edges,calltype,signature)
  importedLibraries=@($imports.imports | ForEach-Object { $_.libname } | Sort-Object -Unique)
  notes=@('This report analyzes the user-owned native executable only. It is not proprietary data committed to the repository.','Function names are native binary labels; source-to-function reconciliation is the next annotation pass.')
}
$full=[IO.Path]::GetFullPath($Output); New-Item -ItemType Directory -Force -Path (Split-Path -Parent $full) | Out-Null
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full"; Write-Output "Analyzed $(@($functions).Count) functions"
