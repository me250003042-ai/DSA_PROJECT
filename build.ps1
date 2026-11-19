# Build all programs using MinGW g++ from PowerShell.
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
New-Item -ItemType Directory -Force build | Out-Null
$programs = @{
    'diff_test' = 'oracle/diff_test.cpp'
    'monitor' = 'app/monitor.cpp'
    'demoA' = 'src/structA/medianA.cpp'
    'demoB' = 'src/structB/medianB.cpp'
    'bench_time' = 'bench/bench.cpp'
}
foreach ($name in $programs.Keys) {
    $flags = @('-std=c++17', '-O2', '-Wall', '-Wextra')
    & g++ @flags $programs[$name] -o "build/$name.exe" -lpsapi
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $name" }
}
Write-Host 'Build complete.'
