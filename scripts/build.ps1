$ErrorActionPreference = "Stop"

$cmake = "C:\Program Files\CMake\bin\cmake.exe"
if (-not (Test-Path $cmake)) {
    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeCmd) {
        $cmake = $cmakeCmd.Source
    } else {
        throw "CMake not found"
    }
}

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw "MSVC toolchain not found. Install Visual Studio C++ workload or Build Tools."
}

$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root "build"
New-Item -ItemType Directory -Force -Path $build | Out-Null

& $cmake -S $root -B $build -G "Visual Studio 18 2026" -A x64
if ($LASTEXITCODE -ne 0) {
    & $cmake -S $root -B $build -G "Visual Studio 17 2022" -A x64
}
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed"
}

& $cmake --build $build --config Release
if ($LASTEXITCODE -ne 0) {
    throw "Build failed"
}

Write-Host "Built: $build\Release\Wakbg.exe"
