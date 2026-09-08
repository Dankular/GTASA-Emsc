[CmdletBinding()]
param(
  [string]$SdkRoot = (Join-Path $PSScriptRoot 'plugin-sdk\plugin_sa\game_sa'),
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\plugin-sdk-map.json')
)
$ErrorActionPreference='Stop'
$SdkRoot=(Resolve-Path -LiteralPath $SdkRoot).Path
$files=@(Get-ChildItem -LiteralPath $SdkRoot -Recurse -File -Include *.h,*.cpp | ForEach-Object FullName)
function Find-Sdk([string]$Pattern){
  $hits=@(Select-String -Path $files -Pattern $Pattern -AllMatches)
  [ordered]@{ matches=$hits.Count; files=@($hits|ForEach-Object{$_.Path.Replace($SdkRoot+'\','')}|Sort-Object -Unique|Select-Object -First 80) }
}
$commit=(git -C (Join-Path $PSScriptRoot 'plugin-sdk') rev-parse HEAD).Trim()
$report=[ordered]@{
  generatedAt=(Get-Date).ToUniversalTime().ToString('o'); sdkRoot=$SdkRoot
  repository='https://github.com/DK22Pac/plugin-sdk'; commit=$commit; sourceFiles=$files.Count
  subsystems=[ordered]@{
    vehicles=Find-Sdk 'CAutomobile|CVehicle|CBike|CBoat|CBmx|CTrain|CarCtrl|CarAI'
    interiors=Find-Sdk 'Interior|CEntryExit|CBuilding|CWorld|ColStore'
    audio=Find-Sdk 'CAEAudio|CAudioEngine|CAEStream|CAERadio|CAESound'
    input=Find-Sdk 'CPad|CController|Mouse|Keyboard|Joy'
    streaming=Find-Sdk 'CStreaming|CdStream|CModelInfo|IMG|RwStream'
    saves=Find-Sdk 'CGenericGameStorage|Save|Load|DataStorage'
    camera=Find-Sdk 'CCamera|CCam'
    scripts=Find-Sdk 'CRunningScript|CTheScripts|Script|Command'
  }
  contract=[ordered]@{ runtime='SDK class/layout reference only'; assets='user-owned original SA assets'; injection='not required by GTASA-Emsc'; next='Implement portable adapters against these class contracts, then add behavior fixtures.' }
}
$full=[IO.Path]::GetFullPath($Output);New-Item -ItemType Directory -Force -Path (Split-Path -Parent $full)|Out-Null
$report|ConvertTo-Json -Depth 8|Set-Content -LiteralPath $full -Encoding utf8
Write-Output "Wrote $full";Write-Output "Scanned $($files.Count) SDK source files at $commit"
