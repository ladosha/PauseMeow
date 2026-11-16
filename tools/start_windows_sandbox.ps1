[CmdletBinding()]
param(
    [switch]$PrepareOnly
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$artifactRoot = Join-Path $projectRoot "artifacts\windows"
if (-not (Test-Path -LiteralPath $artifactRoot -PathType Container)) {
    throw "Windows artifacts directory not found. Build the portable release first: $artifactRoot"
}

$archive = Get-ChildItem -LiteralPath $artifactRoot -File |
    Where-Object { $_.Name -match '^PauseMeow-[0-9]+\.[0-9]+\.[0-9]+-windows-x64-portable\.zip$' } |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1
if (-not $archive) {
    throw "No versioned PauseMeow portable ZIP found in $artifactRoot"
}
if (-not (Test-Path -LiteralPath (Join-Path $artifactRoot "SHA256SUMS.txt") -PathType Leaf)) {
    throw "SHA256SUMS.txt is missing from $artifactRoot"
}

$escapedArtifactRoot = [System.Security.SecurityElement]::Escape($artifactRoot)
$configuration = @"
<Configuration>
  <VGpu>Enable</VGpu>
  <Networking>Disable</Networking>
  <ClipboardRedirection>Disable</ClipboardRedirection>
  <PrinterRedirection>Disable</PrinterRedirection>
  <MappedFolders>
    <MappedFolder>
      <HostFolder>$escapedArtifactRoot</HostFolder>
      <SandboxFolder>C:\PauseMeowRelease</SandboxFolder>
      <ReadOnly>true</ReadOnly>
    </MappedFolder>
  </MappedFolders>
  <LogonCommand>
    <Command>explorer.exe C:\PauseMeowRelease</Command>
  </LogonCommand>
</Configuration>
"@

$configPath = Join-Path $artifactRoot "PauseMeow-Release-Test.wsb"
Set-Content -LiteralPath $configPath -Value $configuration -Encoding utf8
Write-Host "Prepared Windows Sandbox configuration: $configPath"

if ($PrepareOnly) {
    return
}

$sandboxExecutable = Join-Path $env:WINDIR "System32\WindowsSandbox.exe"
$sandboxAssociation = Get-ItemProperty -LiteralPath "Registry::HKEY_CLASSES_ROOT\.wsb" `
    -ErrorAction SilentlyContinue
$legacySandboxAvailable = Test-Path -LiteralPath $sandboxExecutable -PathType Leaf
if (-not $legacySandboxAvailable -and -not $sandboxAssociation) {
    throw "Windows Sandbox is not enabled. Enable Containers-DisposableClientVM and restart Windows first."
}

Start-Process -FilePath $configPath
