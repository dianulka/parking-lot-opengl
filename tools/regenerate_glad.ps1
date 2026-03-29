# Regenerates third_party/glad for OpenGL 3.3 Core (requires: pip install glad2)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
python -m pip install --quiet glad2
python -m glad --api gl:core=3.3 --out-path third_party/glad --reproducible c
Write-Host "Updated third_party/glad — commit the changes."
