[CmdletBinding()]
param(
    [ValidateSet("AppImage", "Flatpak", "All")]
    [string]$Format = "All"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))

if ($Format -in @("AppImage", "All")) {
    if (-not (Get-Command docker.exe -ErrorAction SilentlyContinue)) {
        throw "Docker Desktop with Linux containers is required for the AppImage build."
    }

    docker build `
        --file (Join-Path $projectRoot "packaging\linux\Dockerfile.appimage") `
        --tag pausemeow-appimage-builder `
        $projectRoot
    if ($LASTEXITCODE -ne 0) {
        throw "The AppImage builder image failed to build."
    }

    docker run --rm `
        --volume "${projectRoot}:/workspace" `
        pausemeow-appimage-builder
    if ($LASTEXITCODE -ne 0) {
        throw "The AppImage build failed."
    }
}

if ($Format -in @("Flatpak", "All")) {
    if (-not (Get-Command docker.exe -ErrorAction SilentlyContinue)) {
        throw "Docker Desktop with Linux containers is required for the Flatpak build."
    }

    docker run --rm --privileged `
        --volume "${projectRoot}:/workspace" `
        --workdir /workspace `
        --entrypoint bash `
        ghcr.io/flathub-infra/flatpak-github-actions:kde-6.11 `
        packaging/linux/build-flatpak.sh
    if ($LASTEXITCODE -ne 0) {
        throw "The Flatpak build failed."
    }
}

Write-Host "Linux artifacts are available in artifacts\linux."
