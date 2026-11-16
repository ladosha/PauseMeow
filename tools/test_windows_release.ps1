[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$QtBin,
    [ValidateSet("Release", "RelWithDebInfo")]
    [string]$Configuration = "Release",
    [string]$BuildDir = "",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$QtBin = [System.IO.Path]::GetFullPath($QtBin)
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $projectRoot "build"
}
$BuildDir = [System.IO.Path]::GetFullPath($BuildDir)

$requiredQtFiles = @("Qt6Core.dll", "windeployqt.exe")
foreach ($requiredFile in $requiredQtFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path $QtBin $requiredFile) -PathType Leaf)) {
        throw "Qt runtime file not found in -QtBin: $requiredFile"
    }
}

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList
    )

    & $FilePath @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $FilePath"
    }
}

$originalPath = $env:Path
try {
    $env:Path = "$QtBin;$originalPath"

    if (-not $SkipBuild) {
        Invoke-Checked -FilePath "cmake" -ArgumentList @(
            "--build", $BuildDir, "--config", $Configuration, "--parallel"
        )
    }

    Invoke-Checked -FilePath "ctest" -ArgumentList @(
        "--test-dir", $BuildDir,
        "-C", $Configuration,
        "--output-on-failure"
    )

    $packageArguments = @(
        "-NoProfile", "-ExecutionPolicy", "Bypass",
        "-File", (Join-Path $PSScriptRoot "package_windows.ps1"),
        "-BuildDir", $BuildDir,
        "-Configuration", $Configuration,
        "-QtBin", $QtBin,
        "-SkipBuild",
        "-AllowUnsigned"
    )
    Invoke-Checked -FilePath "powershell.exe" -ArgumentList $packageArguments

    Invoke-Checked -FilePath "powershell.exe" -ArgumentList @(
        "-NoProfile", "-ExecutionPolicy", "Bypass",
        "-File", (Join-Path $PSScriptRoot "test_windows_portable.ps1")
    )
} finally {
    $env:Path = $originalPath
}

Write-Host "Windows release build, tests, packaging, and validation passed."
