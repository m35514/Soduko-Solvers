param(
    [string]$Compiler = 'C:/Program Files/CodeBlocks/MinGW/bin/g++.exe'
)
$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$destination = Join-Path $project 'output/snapshot_benchmark'
New-Item -ItemType Directory -Force $destination | Out-Null
$rows = Import-Csv (Join-Path $project 'results/master_collection_benchmark.csv')
$selected = @{}
# Select the ten most branching puzzles for EACH technique set; deduplicate
# by puzzle string. Include three zero-guess controls from Solver 3.
foreach ($solver in @('Solver3_bitmask', 'Solver4_bitmask', 'Solver5_bitmask')) {
    $heavy = $rows | Where-Object { $_.Solver -eq $solver } |
        Sort-Object { [long]$_.Guesses } -Descending | Select-Object -First 10
    foreach ($row in $heavy) { $selected[$row.'Puzzle String'] = $row.Puzzle }
}
$controls = $rows | Where-Object { $_.Solver -eq 'Solver3_bitmask' -and [long]$_.Guesses -eq 0 } |
    Select-Object -First 3
foreach ($row in $controls) { $selected[$row.'Puzzle String'] = $row.Puzzle }
$manifest = Join-Path $destination 'manifest.tsv'
$selected.GetEnumerator() | Sort-Object Value | ForEach-Object { "$($_.Value)`t$($_.Key)" } |
    Set-Content -Encoding ascii $manifest
$sources = @('tools/snapshot_cost_benchmark.cpp', 'src/solver3.cpp', 'src/solver4.cpp', 'src/solver5.cpp',
    'src/arraySolver3.cpp', 'src/arraySolver4.cpp', 'src/arraySolver5.cpp',
    'src/new stuff/HeuristicEngine.cpp', 'src/new stuff/cellSelection.cpp',
    'src/new stuff/nakedSingle.cpp', 'src/new stuff/hiddenSingle.cpp',
    'src/new stuff/nakedPair.cpp', 'src/new stuff/pointingPair.cpp', 'src/new stuff/xWing.cpp') |
    ForEach-Object { Join-Path $project $_ }
& $Compiler --version | Set-Content (Join-Path $destination 'compiler.txt')
foreach ($mode in @('baseline', 'profile')) {
    $executable = Join-Path $destination "$mode.exe"
    $flags = @('-std=c++17', '-O3', '-march=native', '-Wall', '-Wextra', '-static')
    if ($mode -eq 'profile') { $flags += '-DSUDOKU_PROFILE_SNAPSHOTS' }
    & $Compiler @flags @sources '-o' $executable
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $mode" }
    & $executable $manifest 1> (Join-Path $destination "$mode.csv") 2> (Join-Path $destination "$mode.log")
    if ($LASTEXITCODE -ne 0) { throw "Benchmark failed: $mode" }
}
foreach ($repeat in @('baseline_repeat', 'baseline_repeat2')) {
    & (Join-Path $destination 'baseline.exe') $manifest 1> (Join-Path $destination "$repeat.csv") 2> (Join-Path $destination "$repeat.log")
    if ($LASTEXITCODE -ne 0) { throw "Repeat benchmark failed: $repeat" }
}
$regression = Join-Path $destination 'statistics_test.exe'
$regressionSources = $sources | Select-Object -Skip 1
& $Compiler '-std=c++17' '-O3' '-march=native' '-Wall' '-Wextra' '-static' '-DSUDOKU_PROFILE_SNAPSHOTS' "-I$(Join-Path $project 'src')" (Join-Path $project 'tests/solve_statistics_test.cpp') (Join-Path $project 'src/trace/jsonWriter.cpp') @regressionSources '-o' $regression
if ($LASTEXITCODE -ne 0) { throw 'Regression build failed' }
& $regression (Join-Path $project 'src/data/sudokuwiki_examples.txt') 1> (Join-Path $destination 'statistics_test.csv') 2> (Join-Path $destination 'statistics_test.log')
if ($LASTEXITCODE -ne 0) { throw 'Regression failed' }
& (Join-Path $PSScriptRoot 'summarize_snapshot_benchmark.ps1')
Write-Output "Benchmark complete: $destination"
