$ErrorActionPreference='Stop'
$root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$fixture=Join-Path $root 'reports\baseline-fixture'; New-Item -ItemType Directory -Force -Path $fixture | Out-Null
Set-Content (Join-Path $fixture 'test.img') 'fixture' -NoNewline
$report=Join-Path $root 'reports\baseline-fixture.json'
& (Join-Path $PSScriptRoot 'capture-native-baseline.ps1') -Executable $env:ComSpec -StartupExecutable $env:ComSpec -AssetRoot $fixture -Output $report -StartupTimeoutMs 5000
$json=Get-Content $report -Raw | ConvertFrom-Json
if ($json.schema -ne 'gta-uno.native-baseline/v1' -or $json.assets.count -ne 1 -or !$json.reproducibility.contentDigest) { throw 'baseline evidence assertions failed' }
if (!$json.startup.attempted -or !$json.startup.started -or $json.startup.timedOut) { throw 'startup capture assertions failed' }
$first=$json.reproducibility.contentDigest
& (Join-Path $PSScriptRoot 'capture-native-baseline.ps1') -Executable $env:ComSpec -StartupExecutable $env:ComSpec -AssetRoot $fixture -Output $report -StartupTimeoutMs 5000 -SkipStartup
$second=(Get-Content $report -Raw | ConvertFrom-Json).reproducibility.contentDigest
if ($first -ne $second) { throw 'content digest is not reproducible' }
Write-Output 'baseline evidence test passed'
