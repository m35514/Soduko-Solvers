$ErrorActionPreference = 'Stop'
$destination = Join-Path (Split-Path $PSScriptRoot -Parent) 'output/snapshot_benchmark'
function Format-Number([double]$value, [string]$format = 'F3') {
    $value.ToString($format, [Globalization.CultureInfo]::InvariantCulture)
}
$baseline = @(Import-Csv (Join-Path $destination 'baseline.csv'))
$profile = @(Import-Csv (Join-Path $destination 'profile.csv'))
$identities = @($baseline | ForEach-Object { "$($_.puzzle),$($_.solver),$($_.guesses),$($_.dead_ends),$($_.solution)" })
$lines = [Collections.Generic.List[string]]::new()
$lines.Add('# Backtracking snapshot cost experiment')
$lines.Add('')
$lines.Add('Date: 12 October 2026. Compiler: GCC 14.2.0, C++17, -O3 -march=native -static, Windows x64. Traces and statistics collection are disabled during timed solves. Instrumentation is enabled only with SUDOKU_PROFILE_SNAPSHOTS; normal builds retain their original snapshot operations.')
$lines.Add('')
$lines.Add('## Finding')
$lines.Add('')
$lines.Add('The measurements do not support snapshot copying as the main explanation for an array advantage on heavy puzzles. Copy intervals, even including clock overhead, total less than 1% of baseline solving time in the profiled sample. Warm-cache copy estimates are smaller still. The historical array advantage was not consistently reproduced in the new full-solve runs. This does not establish which other operation explains historical timings.')
$lines.Add('')
$lines.Add('## Method and validation')
$lines.Add('')
$lines.Add('Select the ten highest-guess puzzles for each of bitmask Solvers 3, 4, and 5 from the existing master collection CSV, deduplicate puzzle strings, then add three zero-guess controls. This yields 13 branching puzzles and 3 controls. Run all six array/bitmask configurations on each puzzle with 3 warmups and 31 timed samples; rotate and reverse solver order. Report each puzzle/configuration median. Repeat the uninstrumented run three times. Selection uses old guess counts, not new timing results; this is a targeted sample, not a representative population estimate.')
$lines.Add('')
$lines.Add('Each solve must preserve givens and satisfy all Sudoku constraints. Array and bitmask versions must produce identical solutions, guesses and dead ends. Profile counters must equal guesses (saves) and dead ends (restores). Compare solution/count fields between all builds and runs. The existing 72-case regression suite passes with profiling enabled, including trace/statistics consistency and unsolvable cases.')
$lines.Add('')
$lines.Add('## Full solving times')
$lines.Add('')
$lines.Add('Times below are sums of per-puzzle medians over the 13 branching puzzles, not per-puzzle averages or median speedups. Array/bitmask below 1 means array is faster.')
$lines.Add('')
$lines.Add('| Run | Solver | Bitmask total ms | Array total ms | Array/bitmask |')
$lines.Add('|---|---|---:|---:|---:|')
foreach ($run in @('baseline', 'baseline_repeat', 'baseline_repeat2')) {
    $rows = @(Import-Csv (Join-Path $destination "$run.csv"))
    $other = @($rows | ForEach-Object { "$($_.puzzle),$($_.solver),$($_.guesses),$($_.dead_ends),$($_.solution)" })
    if (Compare-Object $identities $other) { throw "Results changed: $run" }
    foreach ($solver in 3..5) {
        $bm = ($rows | Where-Object { $_.solver -eq "Solver${solver}_bitmask" -and [long]$_.guesses -gt 0 } | Measure-Object median_ms -Sum).Sum
        $ar = ($rows | Where-Object { $_.solver -eq "Solver${solver}_array" -and [long]$_.guesses -gt 0 } | Measure-Object median_ms -Sum).Sum
        $lines.Add("| $run | $solver | $(Format-Number $bm) | $(Format-Number $ar) | $(Format-Number ($ar/$bm) 'F4') |")
    }
}
$other = @($profile | ForEach-Object { "$($_.puzzle),$($_.solver),$($_.guesses),$($_.dead_ends),$($_.solution)" })
if (Compare-Object $identities $other) { throw 'Profiling changed results' }
$lines.Add('')
$lines.Add('## In-solver copy timing')
$lines.Add('')
$lines.Add('Use a separate profiled build to time the actual save and restore statements. Values are raw observed intervals, include timer overhead, and are compared to uninstrumented baseline times. They are diagnostic measurements, not a precise counterfactual speedup from removing copies.')
$lines.Add('')
$lines.Add('| Solver | Saves | Restores | Raw copy ms | Raw copy / baseline time |')
$lines.Add('|---|---:|---:|---:|---:|')
foreach ($group in ($baseline | Where-Object { [long]$_.guesses -gt 0 } | Group-Object solver)) {
    $matches = @($profile | Where-Object { $_.solver -eq $group.Name -and [long]$_.guesses -gt 0 })
    $ms = ($group.Group | Measure-Object median_ms -Sum).Sum
    $ns = ($matches | ForEach-Object { [double]$_.save_ns_raw + [double]$_.restore_ns_raw } | Measure-Object -Sum).Sum
    $saves = ($group.Group | Measure-Object guesses -Sum).Sum
    $restores = ($group.Group | Measure-Object dead_ends -Sum).Sum
    $lines.Add("| $($group.Name) | $saves | $restores | $(Format-Number ($ns/1e6) 'F4') | $(Format-Number (100*$ns/1e6/$ms))% |")
}
$lines.Add('')
$lines.Add('Profiler timer calibration and independent warm-cache microbenchmarks (nanoseconds per board-plus-grid save or restore):')
$lines.Add('')
$lines.Add('```text')
foreach ($run in @('baseline', 'profile', 'baseline_repeat', 'baseline_repeat2')) {
    $lines.Add($run)
    foreach ($line in (Get-Content (Join-Path $destination "$run.log") | Where-Object { $_ -match '^(micro|empty_clock)' })) { $lines.Add($line) }
}
$lines.Add('```')
$lines.Add('')
$lines.Add('A Board occupies 324 bytes; BitmaskCandidateGrid occupies 552 bytes and includes a separate BitmaskBoard; ArrayCandidateGrid occupies 824 bytes. Object size alone does not predict optimized copy time. Microbenchmarks use compiler memory barriers to prevent dead-copy elimination and median batches of 1,000,000 copies. These copies are warm-cache and lack recursive stack/cache behaviour, so they are estimates rather than direct attribution. Individual clock intervals are quantized at about 100 ns on this host; per-copy timings are too small to interpret precisely with that clock. No timer-overhead subtraction is used to claim a precise copy cost.')
$lines.Add('')
$lines.Add('## Limits and conclusion')
$lines.Add('')
$lines.Add('Run-to-run total times vary, so a small representation advantage is not robust evidence of a particular mechanism. The controlled microbenchmark sometimes favours array copies, but its estimated saving is much smaller than millisecond-scale solving costs. We have measured the implemented mechanism; we have not performed a copy-free solver ablation. A causal claim would need a validated alternative restoration strategy and a larger, controlled comparison. The justified report wording is: "Snapshot copying occurs in both implementations, but targeted profiling found it contributed a small fraction of runtime and did not support it as the main cause of an array advantage."')
$lines.Add('')
$lines.Add('## Reproduce')
$lines.Add('')
$lines.Add('Run tools/run_snapshot_benchmark.ps1 (optionally supply -Compiler with a GCC path), then tools/summarize_snapshot_benchmark.ps1. Sources: tools/snapshot_cost_benchmark.cpp, src/new stuff/snapshotProfile.hpp, and the opt-in sections of src/new stuff/HeuristicEngine.cpp. Raw CSVs, manifests and logs are in this folder.')
$lines | Set-Content -Encoding utf8 (Join-Path $destination 'report.md')
Write-Output "Verified 96 solution/count pairs across all four runs. Report: $destination/report.md"
