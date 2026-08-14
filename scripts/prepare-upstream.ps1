param(
    [Parameter(Mandatory = $true)]
    [string]$Destination
)

$ErrorActionPreference = 'Stop'
$upstream = 'https://github.com/reald/uv-k5-firmware-custom.git'
$commit = '5955ccfc8732f4a16b628276ed5fa98f2db54e55'
$repoRoot = Split-Path -Parent $PSScriptRoot
$overlay = Join-Path $repoRoot 'source'
$destinationPath = [System.IO.Path]::GetFullPath($Destination)

if (Test-Path -LiteralPath $destinationPath) {
    throw "Destination already exists: $destinationPath"
}

git clone $upstream $destinationPath
if ($LASTEXITCODE -ne 0) { throw 'Cloning the upstream repository failed.' }

git -C $destinationPath checkout $commit
if ($LASTEXITCODE -ne 0) { throw 'Checking out the required upstream commit failed.' }

Copy-Item -Path (Join-Path $overlay '*') -Destination $destinationPath -Recurse -Force

Write-Host "Prepared source tree: $destinationPath"
Write-Host 'Build with: make ENABLE_BEACON_MO=1 ENABLE_PREVENT_TX=0'
