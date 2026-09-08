[CmdletBinding()]
param(
  [string]$Executable = 'C:\Program Files (x86)\Rockstar Games\GTA San Andreas\gta_sa_compact1.0.exe',
  [string]$StartupExecutable = 'C:\Program Files (x86)\Rockstar Games\GTA San Andreas\gta_sa.exe',
  [string]$AssetRoot = 'C:\Program Files (x86)\Rockstar Games\GTA San Andreas',
  [string]$Output = (Join-Path $PSScriptRoot '..\reports\native-baseline.json'),
  [int]$StartupTimeoutMs = 30000,
  [switch]$SkipStartup
)
$ErrorActionPreference = 'Stop'

function Get-FileEvidence([string]$path, [string]$root) {
  $item = Get-Item -LiteralPath $path
  if ($root) {
    $rootUri=[Uri]((Resolve-Path -LiteralPath $root).Path.TrimEnd('\')+'\')
    $relative=$rootUri.MakeRelativeUri([Uri]$item.FullName).ToString().Replace('%20',' ').Replace('\','/')
  } else { $relative=$item.Name }
  [ordered]@{ path=$relative; size=[int64]$item.Length; sha256=(Get-FileHash -LiteralPath $item.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
}

function Get-AssetEvidence([string]$root) {
  if (!(Test-Path -LiteralPath $root -PathType Container)) { return @() }
  $extensions = @('.img','.txd','.dff','.col','.ifp','.ipl','.ide','.dat','.scm','.awb','.wav','.mp3','.ogg')
  $files = Get-ChildItem -LiteralPath $root -File -Recurse | Where-Object { $extensions -contains $_.Extension.ToLowerInvariant() } | Sort-Object FullName
  @($files | ForEach-Object { Get-FileEvidence $_.FullName $root })
}

function Get-ContentDigest($files) {
  $canonical = (@($files | ForEach-Object { "$($_.path)|$($_.size)|$($_.sha256)" }) -join "`n")
  $bytes = [Text.Encoding]::UTF8.GetBytes($canonical)
  ([Security.Cryptography.SHA256]::Create().ComputeHash($bytes) | ForEach-Object { $_.ToString('x2') }) -join ''
}

function Capture-Startup([string]$path, [int]$timeoutMs, [string]$logRoot) {
  $started = [DateTime]::UtcNow
  $stamp = $started.ToString('yyyyMMddTHHmmssfffZ')
  $stdoutPath = Join-Path $logRoot "startup-$stamp.stdout.log"
  $stderrPath = Join-Path $logRoot "startup-$stamp.stderr.log"
  $checkpointPath = Join-Path $logRoot "startup-$stamp.checkpoints.jsonl"
  $events = [Collections.Generic.List[object]]::new()
  function Add-Checkpoint([string]$name, [hashtable]$data=@{}) {
    $event = [ordered]@{ sequence=$events.Count; checkpoint=$name; utc=[DateTime]::UtcNow.ToString('o'); data=$data }
    $events.Add($event)
    ($event | ConvertTo-Json -Compress -Depth 8) | Add-Content -LiteralPath $checkpointPath -Encoding utf8
  }
  Add-Checkpoint 'launch-requested' @{ executable=$path; timeoutMs=$timeoutMs }
  $psi = [Diagnostics.ProcessStartInfo]::new()
  $psi.FileName=$path; $psi.WorkingDirectory=(Split-Path -Parent $path)
  $psi.UseShellExecute=$false; $psi.CreateNoWindow=$true
  $psi.RedirectStandardOutput=$true; $psi.RedirectStandardError=$true
  $process = [Diagnostics.Process]::new(); $process.StartInfo=$psi
  try {
    if (!$process.Start()) { throw 'Process.Start returned false' }
    Add-Checkpoint 'process-started' @{ pid=$process.Id }
    $outTask=$process.StandardOutput.ReadToEndAsync(); $errTask=$process.StandardError.ReadToEndAsync()
    $exited=$process.WaitForExit($timeoutMs)
    if (!$exited) {
      Add-Checkpoint 'startup-timeout' @{ pid=$process.Id; timeoutMs=$timeoutMs }
      try { $process.Kill() } catch { }
      $process.WaitForExit()
    } else { Add-Checkpoint 'process-exited' @{ exitCode=$process.ExitCode } }
    [IO.File]::WriteAllText($stdoutPath, $outTask.Result, [Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText($stderrPath, $errTask.Result, [Text.UTF8Encoding]::new($false))
    [ordered]@{ attempted=$true; started=$true; exited=$exited; exitCode=if($exited){$process.ExitCode}else{$null}; timedOut=(-not $exited); pid=$process.Id; stdout=$stdoutPath; stderr=$stderrPath; checkpoints=$checkpointPath; events=@($events) }
  } catch {
    Add-Checkpoint 'launch-failed' @{ error=$_.Exception.Message }
    [ordered]@{ attempted=$true; started=$false; exited=$false; timedOut=$false; exitCode=$null; error=$_.Exception.Message; stdout=$stdoutPath; stderr=$stderrPath; checkpoints=$checkpointPath; events=@($events) }
  } finally { $process.Dispose() }
}

$fullOutput=[IO.Path]::GetFullPath($Output); $reportRoot=Split-Path -Parent $fullOutput
New-Item -ItemType Directory -Force -Path $reportRoot | Out-Null
$files=@()
if (Test-Path -LiteralPath $Executable -PathType Leaf) { $files += Get-FileEvidence $Executable '' }
if ($StartupExecutable -and (Test-Path -LiteralPath $StartupExecutable -PathType Leaf) -and $StartupExecutable -ne $Executable) { $files += Get-FileEvidence $StartupExecutable '' }
$assets=@(Get-AssetEvidence $AssetRoot)
$startup=[ordered]@{ attempted=$false; reason='-SkipStartup supplied' }
if (!$SkipStartup -and (Test-Path -LiteralPath $StartupExecutable -PathType Leaf)) { $startup=Capture-Startup $StartupExecutable $StartupTimeoutMs $reportRoot }
elseif (!$SkipStartup) { $startup=[ordered]@{ attempted=$false; reason="startup executable not found: $StartupExecutable" } }
$report=[ordered]@{
  schema='gta-uno.native-baseline/v1'; generatedAt=(Get-Date).ToUniversalTime().ToString('o')
  reproducibility=[ordered]@{ hashAlgorithm='SHA256'; assetOrdering='relative path ascending'; contentDigest=(Get-ContentDigest ($files+$assets)); proprietaryBytesStored=$false }
  executables=@($files); assets=[ordered]@{ root=$AssetRoot; count=@($assets).Count; files=@($assets); contentDigest=(Get-ContentDigest $assets) }
  startup=$startup
}
$report | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $fullOutput -Encoding utf8
Write-Output "Wrote $fullOutput"; Write-Output "Evidence files: $($files.Count); assets: $($assets.Count); content digest: $($report.reproducibility.contentDigest)"
if ($startup.attempted -and (!$startup.started -or $startup.timedOut)) { exit 1 }
