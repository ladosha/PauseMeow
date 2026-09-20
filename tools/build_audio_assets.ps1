param(
    [string]$Ffmpeg = "ffmpeg"
)

$ErrorActionPreference = "Stop"

$audioDirectory = Join-Path $PSScriptRoot "..\assets\audio"
New-Item -ItemType Directory -Path $audioDirectory -Force | Out-Null

$stepsPath = Join-Path $audioDirectory "cat_steps.ogg"
$stepsExpression = "0.20*(sin(2*PI*100*t)+0.30*sin(2*PI*200*t)+0.12*sin(2*PI*400*t))*exp(-45*mod(t+0.28\,0.36))"
& $Ffmpeg -hide_banner -loglevel error -y `
    -f lavfi -i "aevalsrc=$stepsExpression`:s=48000" `
    -t 0.72 -ac 1 -c:a libvorbis -q:a 6 $stepsPath
if ($LASTEXITCODE -ne 0) {
    throw "ffmpeg failed to generate $stepsPath"
}

Get-Item $stepsPath
