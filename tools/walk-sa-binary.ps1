[CmdletBinding()]
param(
  [string]$Executable = 'C:\Program Files (x86)\Rockstar Games\GTA San Andreas\gta_sa_compact1.0.exe',
  [string]$RzBin = 'D:\Dev Proj\GTA Uno\GTASA\rizin-static\rizin-win-installer-vs2019_static-64\bin\rz-bin.exe',
  [string]$Rizin = 'D:\Dev Proj\GTA Uno\GTASA\rizin-static\rizin-win-installer-vs2019_static-64\bin\rizin.exe',
  [string]$Compass = (Join-Path $PSScriptRoot 'binary\build\src\cli\compass-cli.exe'),
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\sa-binary-walk.json')
)
$ErrorActionPreference='Stop'
foreach($path in @($Executable,$RzBin,$Rizin)){if(!(Test-Path -LiteralPath $path)){throw "Missing input/tool: $path"}}
function Read-JsonCommand([string]$tool,[string[]]$args){$raw=& $tool @args 2>$null|Out-String;try{return $raw|ConvertFrom-Json}catch{return $null}}
$infoRaw=& $RzBin '-Ij' $Executable 2>$null|Out-String
$importsRaw=& $RzBin '-ij' $Executable 2>$null|Out-String
$sectionsRaw=& $RzBin '-sj' $Executable 2>$null|Out-String
$info=($infoRaw|ConvertFrom-Json).info
$imports=($importsRaw|ConvertFrom-Json).imports
$sections=($sectionsRaw|ConvertFrom-Json).sections
$functionsRaw=& $Rizin '-2' '-q' '-c' 'aa; aflj; q' $Executable 2>$null|Out-String
$functions=try{$functionsRaw|ConvertFrom-Json}catch{@()}
$patterns=[ordered]@{
  renderer='Direct3D|DirectDraw|RenderWare|Rw[A-Z]|d3d9|DDRAW'
  audio='DirectSound|DSOUND|WINMM|vorbis|EAX|Audio|AE[A-Z]'
  input='DirectInput|DINPUT|USER32|Pad|Mouse|Keyboard'
  platform='KERNEL32|ADVAPI32|USER32|GDI32|CreateFile|VirtualProtect|LoadLibrary'
  streaming='IMG|CdStream|CStreaming|Stream|ReadFile|SetFilePointer'
  networking='WS2_32|socket|connect|recv|send'
}
$libs=@($imports|ForEach-Object{$_.libname}|Sort-Object -Unique)
$classified=[ordered]@{}
foreach($name in $patterns.Keys){$classified[$name]=@($libs|Where-Object{$_ -match $patterns[$name]})}
$report=[ordered]@{
 generatedAt=(Get-Date).ToUniversalTime().ToString('o'); executable=(Resolve-Path $Executable).Path
 sha256=(Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
 analyzer=[ordered]@{backend='Rizin-compatible Binary walker'; compassPresent=(Test-Path -LiteralPath $Compass); compassPath=$Compass}
 info=$info; importedLibraries=$libs; importedFunctions=@($imports|Select-Object name,libname,plt); sections=@($sections|Select-Object name,vaddr,size,perm)
 functionCount=@($functions).Count; functions=@($functions|Select-Object name,offset,size,nbbs,edges,calltype,signature); classifiedBoundaries=$classified
 implementationRule='Use this report to replace one boundary at a time in runtime/. Do not copy proprietary bytes or execute original addresses in WASM.'
}
$full=[IO.Path]::GetFullPath($Output);New-Item -ItemType Directory -Force -Path (Split-Path -Parent $full)|Out-Null;$report|ConvertTo-Json -Depth 10|Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full";Write-Output "Functions discovered: $(@($functions).Count); imports: $($libs.Count)"
