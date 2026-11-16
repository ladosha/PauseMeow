[CmdletBinding()]
param(
    [string]$ArchivePath = "",
    [string]$ChecksumPath = "",
    [switch]$KeepExtracted
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$artifactRoot = Join-Path $projectRoot "artifacts\windows"
$cmakeLists = Get-Content (Join-Path $projectRoot "CMakeLists.txt") -Raw
$versionMatch = [regex]::Match(
    $cmakeLists,
    'project\(PauseMeow VERSION ([0-9]+\.[0-9]+\.[0-9]+)')
if (-not $versionMatch.Success) {
    throw "Could not read the PauseMeow version from CMakeLists.txt."
}

$packageName = "PauseMeow-$($versionMatch.Groups[1].Value)-windows-x64-portable"
if ([string]::IsNullOrWhiteSpace($ArchivePath)) {
    $ArchivePath = Join-Path $artifactRoot "$packageName.zip"
}
if ([string]::IsNullOrWhiteSpace($ChecksumPath)) {
    $ChecksumPath = Join-Path $artifactRoot "SHA256SUMS.txt"
}
$ArchivePath = [System.IO.Path]::GetFullPath($ArchivePath)
$ChecksumPath = [System.IO.Path]::GetFullPath($ChecksumPath)

if (-not (Test-Path -LiteralPath $ArchivePath -PathType Leaf)) {
    throw "Portable archive not found: $ArchivePath"
}
if (-not (Test-Path -LiteralPath $ChecksumPath -PathType Leaf)) {
    throw "Checksum file not found: $ChecksumPath"
}

$checksumLine = Get-Content -LiteralPath $ChecksumPath |
    Where-Object { $_ -match "\s+$([regex]::Escape($packageName)).zip$" } |
    Select-Object -First 1
if (-not $checksumLine) {
    throw "No checksum entry found for $packageName.zip."
}
$expectedHash = ($checksumLine -split '\s+', 2)[0].ToLowerInvariant()

$testRoot = Join-Path ([System.IO.Path]::GetTempPath()) (
    "PauseMeow Release Test " + [guid]::NewGuid().ToString("N"))
$copiedArchive = Join-Path $testRoot "$packageName.zip"
$extractRoot = Join-Path $testRoot "Extracted Package"

try {
    New-Item -ItemType Directory -Path $extractRoot -Force | Out-Null
    Copy-Item -LiteralPath $ArchivePath -Destination $copiedArchive

    $actualHash = (Get-FileHash -LiteralPath $copiedArchive -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actualHash -ne $expectedHash) {
        throw "SHA-256 mismatch. Expected $expectedHash, got $actualHash."
    }

    Expand-Archive -LiteralPath $copiedArchive -DestinationPath $extractRoot
    $packageRoot = Join-Path $extractRoot $packageName
    if (-not (Test-Path -LiteralPath $packageRoot -PathType Container)) {
        throw "Archive does not contain the expected root directory: $packageName"
    }

    $requiredFiles = @(
        "PauseMeow.exe",
        "PORTABLE_README.md",
        "README.md",
        "LICENSE",
        "ASSET_PROVENANCE.md",
        "PRIVACY.md",
        "THIRD_PARTY_NOTICES.md",
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Multimedia.dll",
        "Qt6Qml.dll",
        "Qt6Quick.dll",
        "platforms\qwindows.dll",
        "multimedia\ffmpegmediaplugin.dll",
        "qml\QtQuick\qtquick2plugin.dll"
    )
    foreach ($relativePath in $requiredFiles) {
        $candidate = Join-Path $packageRoot $relativePath
        if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            throw "Required portable-package file is missing: $relativePath"
        }
    }

    $forbiddenPatterns = @(
        '*.pdb', '*.ilk', '*.lib', '*.exp', '*.obj', '*.pfx', '*.p12',
        '*.pem', '*.key', '*.iss'
    )
    foreach ($pattern in $forbiddenPatterns) {
        $match = Get-ChildItem -LiteralPath $packageRoot -Recurse -File -Filter $pattern |
            Select-Object -First 1
        if ($match) {
            throw "Forbidden portable-package file found: $($match.FullName)"
        }
    }

    $forbiddenNames = @('SECURITY.md', 'SUPPORT.md')
    foreach ($name in $forbiddenNames) {
        $match = Get-ChildItem -LiteralPath $packageRoot -Recurse -File -Filter $name |
            Select-Object -First 1
        if ($match) {
            throw "Forbidden portable-package document found: $($match.FullName)"
        }
    }

    $forbiddenDirectories = @('build', 'internal', 'backups', 'installer')
    $badDirectory = Get-ChildItem -LiteralPath $packageRoot -Recurse -Directory |
        Where-Object { $forbiddenDirectories -contains $_.Name.ToLowerInvariant() } |
        Select-Object -First 1
    if ($badDirectory) {
        throw "Forbidden portable-package directory found: $($badDirectory.FullName)"
    }

    $executable = Join-Path $packageRoot "PauseMeow.exe"
    $savedPath = $env:Path
    $savedQtPluginPath = $env:QT_PLUGIN_PATH
    $savedQmlImportPath = $env:QML_IMPORT_PATH
    $savedQml2ImportPath = $env:QML2_IMPORT_PATH
    try {
        $env:Path = "$env:SystemRoot\System32;$env:SystemRoot"
        Remove-Item Env:\QT_PLUGIN_PATH -ErrorAction SilentlyContinue
        Remove-Item Env:\QML_IMPORT_PATH -ErrorAction SilentlyContinue
        Remove-Item Env:\QML2_IMPORT_PATH -ErrorAction SilentlyContinue

        $process = Start-Process -FilePath $executable -ArgumentList "--smoke-test" `
            -WorkingDirectory $packageRoot -WindowStyle Hidden -PassThru
        try {
            Wait-Process -Id $process.Id -Timeout 30 -ErrorAction Stop
        } catch {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
            throw "Packaged smoke test did not exit within 30 seconds."
        }
        $process.Refresh()
        if ($process.ExitCode -ne 0) {
            throw "Packaged smoke test failed with exit code $($process.ExitCode)."
        }
    } finally {
        $env:Path = $savedPath
        $env:QT_PLUGIN_PATH = $savedQtPluginPath
        $env:QML_IMPORT_PATH = $savedQmlImportPath
        $env:QML2_IMPORT_PATH = $savedQml2ImportPath
    }

    Write-Host "Portable package validation passed."
    Write-Host "Archive: $ArchivePath"
    Write-Host "SHA-256: $actualHash"
    Write-Host "Test extraction: $packageRoot"
} finally {
    if (-not $KeepExtracted -and (Test-Path -LiteralPath $testRoot)) {
        $resolvedTestRoot = [System.IO.Path]::GetFullPath($testRoot)
        $resolvedTempRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath())
        $withinTemp = $resolvedTestRoot.StartsWith(
            $resolvedTempRoot,
            [System.StringComparison]::OrdinalIgnoreCase)
        $hasExpectedName = [System.IO.Path]::GetFileName($resolvedTestRoot) -like
            'PauseMeow Release Test *'
        if (-not $withinTemp -or -not $hasExpectedName) {
            throw "Refusing to remove unexpected test directory: $resolvedTestRoot"
        }
        Remove-Item -LiteralPath $resolvedTestRoot -Recurse -Force
    }
}
