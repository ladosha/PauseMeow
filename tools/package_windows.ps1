[CmdletBinding()]
param(
    [string]$BuildDir = "",
    [ValidateSet("Release", "RelWithDebInfo")]
    [string]$Configuration = "Release",
    [string]$QtBin = "",
    [string]$InnoSetupCompiler = "",
    [string]$SignTool = "",
    [string]$CertificatePath = "",
    [string]$CertificateThumbprint = "",
    [string]$TimestampUrl = "http://timestamp.digicert.com",
    [switch]$SkipBuild,
    [switch]$AllowUnsigned
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $projectRoot "build"
}
$BuildDir = [System.IO.Path]::GetFullPath($BuildDir)
$artifactRoot = Join-Path $projectRoot "artifacts\windows"
$stageDir = Join-Path $artifactRoot "DeskCat"
$cmakeLists = Get-Content (Join-Path $projectRoot "CMakeLists.txt") -Raw
$versionMatch = [regex]::Match($cmakeLists, 'project\(DeskCat VERSION ([0-9]+\.[0-9]+\.[0-9]+)')
if (-not $versionMatch.Success) {
    throw "Could not read the DeskCat version from CMakeLists.txt."
}
$appVersion = $versionMatch.Groups[1].Value

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

function Find-SignTool {
    if (-not [string]::IsNullOrWhiteSpace($SignTool)) {
        return $SignTool
    }
    $command = Get-Command "signtool.exe" -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }
    $kitsRoot = Join-Path ${env:ProgramFiles(x86)} "Windows Kits\10\bin"
    if (Test-Path $kitsRoot) {
        $candidate = Get-ChildItem $kitsRoot -Filter "signtool.exe" -Recurse |
            Where-Object { $_.FullName -match '\\x64\\signtool\.exe$' } |
            Sort-Object FullName -Descending |
            Select-Object -First 1
        if ($candidate) {
            return $candidate.FullName
        }
    }
    return ""
}

function Find-Iscc {
    if (-not [string]::IsNullOrWhiteSpace($InnoSetupCompiler)) {
        return $InnoSetupCompiler
    }
    $command = Get-Command "ISCC.exe" -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }
    foreach ($candidate in @(
        (Join-Path $projectRoot ".tools\InnoSetup7\ISCC.exe"),
        (Join-Path $projectRoot ".tools\InnoSetup6\ISCC.exe"),
        (Join-Path $env:ProgramFiles "Inno Setup 7\ISCC.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 7\ISCC.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe"),
        (Join-Path $env:ProgramFiles "Inno Setup 6\ISCC.exe")
    )) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }
    throw "Inno Setup 6 or 7 was not found. Install it or pass -InnoSetupCompiler."
}

function Invoke-AuthenticodeSign {
    param([Parameter(Mandatory = $true)][string]$Path)

    if ($AllowUnsigned) {
        Write-Warning "Leaving unsigned: $Path"
        return
    }

    $resolvedSignTool = Find-SignTool
    if ([string]::IsNullOrWhiteSpace($resolvedSignTool)) {
        throw "signtool.exe was not found. Install the Windows SDK or pass -SignTool."
    }

    $arguments = @("sign", "/fd", "SHA256", "/td", "SHA256", "/tr", $TimestampUrl)
    if (-not [string]::IsNullOrWhiteSpace($CertificatePath)) {
        if (-not (Test-Path -LiteralPath $CertificatePath -PathType Leaf)) {
            throw "Certificate file does not exist: $CertificatePath"
        }
        $password = $env:DESKCAT_SIGN_PASSWORD
        if ([string]::IsNullOrWhiteSpace($password)) {
            throw "Set DESKCAT_SIGN_PASSWORD when signing with a PFX certificate."
        }
        $arguments += @("/f", [System.IO.Path]::GetFullPath($CertificatePath), "/p", $password)
    } elseif (-not [string]::IsNullOrWhiteSpace($CertificateThumbprint)) {
        $arguments += @("/sha1", ($CertificateThumbprint -replace '\s', ''))
    } else {
        throw "Pass -CertificatePath or -CertificateThumbprint, or explicitly use -AllowUnsigned."
    }
    $arguments += $Path
    Invoke-Checked -FilePath $resolvedSignTool -ArgumentList $arguments
    Invoke-Checked -FilePath $resolvedSignTool -ArgumentList @("verify", "/pa", "/v", $Path)
}

if (-not $SkipBuild) {
    Invoke-Checked -FilePath "cmake" -ArgumentList @(
        "--build", $BuildDir, "--config", $Configuration, "--parallel"
    )
}

$builtExe = Join-Path $BuildDir "$Configuration\DeskCat.exe"
if (-not (Test-Path -LiteralPath $builtExe -PathType Leaf)) {
    throw "DeskCat executable was not found: $builtExe"
}

if ([string]::IsNullOrWhiteSpace($QtBin)) {
    $cachePath = Join-Path $BuildDir "CMakeCache.txt"
    if (Test-Path $cachePath) {
        $qtEntry = Get-Content $cachePath |
            Where-Object { $_ -match '^Qt6_DIR:PATH=' } |
            Select-Object -First 1
        if ($qtEntry) {
            $qtCmakeDir = ($qtEntry -split '=', 2)[1]
            $QtBin = [System.IO.Path]::GetFullPath((Join-Path $qtCmakeDir "..\..\..\bin"))
        }
    }
}
$winDeployQt = if ([string]::IsNullOrWhiteSpace($QtBin)) {
    ""
} else {
    Join-Path $QtBin "windeployqt.exe"
}
if ([string]::IsNullOrWhiteSpace($winDeployQt) -or -not (Test-Path $winDeployQt)) {
    throw "windeployqt.exe was not found. Pass the Qt bin directory via -QtBin."
}

New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
if (Test-Path -LiteralPath $stageDir) {
    $resolvedStage = [System.IO.Path]::GetFullPath($stageDir)
    $resolvedArtifactRoot = [System.IO.Path]::GetFullPath($artifactRoot)
    if (-not $resolvedStage.StartsWith($resolvedArtifactRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clear a staging directory outside the artifact root: $resolvedStage"
    }
    Remove-Item -LiteralPath $resolvedStage -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $stageDir | Out-Null

$stageExe = Join-Path $stageDir "DeskCat.exe"
Copy-Item -LiteralPath $builtExe -Destination $stageExe
Invoke-Checked -FilePath $winDeployQt -ArgumentList @(
    "--release", "--qmldir", (Join-Path $projectRoot "qml"), "--dir", $stageDir, $stageExe
)
Invoke-AuthenticodeSign -Path $stageExe

$iscc = Find-Iscc
$installerScript = Join-Path $projectRoot "installer\DeskCat.iss"
Invoke-Checked -FilePath $iscc -ArgumentList @(
    "/DAppVersion=$appVersion",
    "/DSourceDir=$stageDir",
    "/DOutputDir=$artifactRoot",
    $installerScript
)

$installer = Join-Path $artifactRoot "DeskCat-Setup-$appVersion-x64.exe"
if (-not (Test-Path -LiteralPath $installer -PathType Leaf)) {
    throw "The installer was not created at the expected path: $installer"
}
Invoke-AuthenticodeSign -Path $installer

Write-Host "Created and verified: $installer"
