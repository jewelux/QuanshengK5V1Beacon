param(
    [Parameter(Mandatory = $true)]
    [string]$Destination,
    [ValidateSet('V1', 'V3')]
    [string]$Hardware = 'V1'
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$destinationPath = [System.IO.Path]::GetFullPath($Destination)
if (Test-Path -LiteralPath $destinationPath) {
    throw "Destination already exists: $destinationPath"
}

if ($Hardware -eq 'V1') {
    $upstream = 'https://github.com/reald/uv-k5-firmware-custom.git'
    $commit = '5955ccfc8732f4a16b628276ed5fa98f2db54e55'
    $overlay = Join-Path $repoRoot 'source'
} else {
    $upstream = 'https://github.com/reald/uv-k1-k5v3-firmware-custom.git'
    $commit = '97b1890bed9628f625d787bfa42683f2da912614'
    $overlay = Join-Path $repoRoot 'source-v3'
}

git clone $upstream $destinationPath
if ($LASTEXITCODE -ne 0) { throw 'Cloning the upstream repository failed.' }
git -C $destinationPath checkout --detach $commit
if ($LASTEXITCODE -ne 0) { throw 'Checking out the required upstream commit failed.' }
Copy-Item -Path (Join-Path $overlay '*') -Destination $destinationPath -Recurse -Force

if ($Hardware -eq 'V3') {
    $shared = Join-Path $repoRoot 'source/app'
    $target = Join-Path $destinationPath 'App/app'
    Copy-Item -LiteralPath (Join-Path $shared 'beacon.c') -Destination $target -Force
    Copy-Item -LiteralPath (Join-Path $shared 'beacon.h') -Destination $target -Force
}

Write-Host "Prepared $Hardware source tree: $destinationPath"
if ($Hardware -eq 'V1') {
    Write-Host 'Build with: make ENABLE_BEACON_MO=1 ENABLE_PREVENT_TX=0 ENABLE_ARDF=0 ENABLE_SPECTRUM=0 ENABLE_FMRADIO=0'
} else {
    Write-Host 'Build with: cmake --preset Beacon'
    Write-Host 'Then: cmake --build --preset Beacon'
}
