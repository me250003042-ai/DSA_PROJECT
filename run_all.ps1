param([switch]$Quick)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$env:PYTHONUTF8 = '1'

# Stop immediately if a native executable reports failure.
function Run($Program, $Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Failed: $Program" }
}

# Check graph support before starting the experiments.
Run 'python' @('-c', 'import matplotlib')
& ./build.ps1
if ($Quick) {
    Run './build/diff_test.exe' @(50, 100000)
} else {
    Run './build/diff_test.exe' @(300, 1000000)
}
Run './build/demoA.exe' @()
Run './build/demoB.exe' @()
if ($Quick) {
    Run './build/monitor.exe' @(100000, 1000, 110, 1)
} else {
    Run './build/monitor.exe' @(1000000, 1000, 110, 1)
}

if ($Quick) {
    foreach ($structure in @('A', 'B')) {
        Run './build/bench_time.exe' @($structure, 1000, 10, 1000000, 50000, 1, 2.0)
    }
    Write-Host 'Quick run finished. Compare the checksum fields (12th column). Results files were not changed.'
} else {
    Run 'python' @('bench/run_bench.py')
    Write-Host 'Full run finished. CSV files: results/. Graphs: results/figures/.'
}
