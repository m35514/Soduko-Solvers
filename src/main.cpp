#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "datasetLoader.hpp"
#include "treeTraversalVisualizer.hpp"
#include <cstdio>
#include <array>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

// Adjust the filename at the end of this path if yours differs --
// everything before src/data/ is your confirmed CLion project folder.
const std::string DATASET_PATH = R"(C:\Users\malob\CLionProjects\sudokku\src\data\sudoku_cluewise.csv)";

// Sparse puzzles expose real alternative branches in the MRV visualization.
const int VISUAL_MIN_CLUES = 17;
const int VISUAL_MAX_CLUES = 17;
// Denser puzzles keep a multi-puzzle timing run practical.
const int BATCH_MIN_CLUES = 35;
const int BATCH_MAX_CLUES = 45;

const std::string RESULTS_DIRECTORY = R"(C:\Users\malob\CLionProjects\sudokku\results)";
const std::size_t BENCHMARK_WARMUP_RUNS = 5;
const std::size_t BENCHMARK_TIMED_RUNS = 31;

volatile std::uint64_t timingGuard = 0;

struct ResultSummary
{
    std::size_t guesses = 0;
    std::size_t deadEnds = 0;
    std::size_t placements = 0;
    std::size_t eliminations = 0;
};

static ResultSummary summarizeTrace(const SolveResult& result)
{
    ResultSummary summary;
    for (const TraceEvent& event : result.trace)
    {
        summary.guesses += event.isGuess;
        summary.deadEnds += event.isGuess && event.leadsToConflict;
        summary.placements += event.placement.has_value();
        summary.eliminations += event.eliminations.size();
    }
    return summary;
}

static void printResearchTable(const std::array<SolveResult, 5>& results)
{
    static const std::array<const char*, 5> names = {"Naive", "Bitmask", "Solver3", "Solver4", "Solver5"};
    printf("\nResearch comparison (median of %zu trace-free runs after %zu warm-ups):\n", BENCHMARK_TIMED_RUNS, BENCHMARK_WARMUP_RUNS);
    printf("%-9s %12s %9s %10s %12s %12s %13s %10s\n", "Solver", "Time (ms)", "Nodes", "Guesses", "Dead ends", "Placements", "Eliminations", "Events");
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const ResultSummary summary = summarizeTrace(results[index]);
        printf("%-9s %12.6f %9llu %10zu %12zu %12zu %13zu %10zu\n", names[index], results[index].timeMs,
               static_cast<unsigned long long>(results[index].nodesExplored), summary.guesses, summary.deadEnds,
               summary.placements, summary.eliminations, results[index].trace.size());
    }
    printf("Note: Nodes are successful placements that enter a search branch for every solver.\n");
}

static void writeResearchTable(std::ostream& out, const std::array<SolveResult, 5>& results)
{
    static const std::array<const char*, 5> names = {"Naive", "Bitmask", "Solver3", "Solver4", "Solver5"};
    out << "\nResearch comparison (Nodes are successful placements that enter a search branch)\n"
        << "Solver,Time ms,Nodes,Guesses,Dead ends,Placements,Eliminations,Trace events\n";
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const ResultSummary summary = summarizeTrace(results[index]);
        out << names[index] << ',' << results[index].timeMs << ',' << results[index].nodesExplored << ','
            << summary.guesses << ',' << summary.deadEnds << ',' << summary.placements << ','
            << summary.eliminations << ',' << results[index].trace.size() << '\n';
    }
}

template <typename Solver>
static double benchmarkMedian(const Board& board)
{
    for (std::size_t run = 0; run < BENCHMARK_WARMUP_RUNS; ++run)
    {
        timingGuard ^= Solver::solve(board, false).nodesExplored;
    }
    std::array<double, BENCHMARK_TIMED_RUNS> samples{};
    for (std::size_t run = 0; run < BENCHMARK_TIMED_RUNS; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        timingGuard ^= Solver::solve(board, false).nodesExplored;
        const auto end = std::chrono::steady_clock::now();
        samples[run] = std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

static bool ensureResultsDirectory()
{
    return _mkdir(RESULTS_DIRECTORY.c_str()) == 0 || errno == EEXIST;
}

static bool ensureDirectory(const std::string& path)
{
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

int main(int argc, char* argv[])
{
    const bool isBatch = argc >= 2 && std::string(argv[1]) == "batch";
    const unsigned int seed = isBatch
        ? (argc >= 4 ? static_cast<unsigned int>(std::stoul(argv[3])) : DatasetLoader::randomSeed())
        : (argc >= 2 ? static_cast<unsigned int>(std::stoul(argv[1])) : DatasetLoader::randomSeed());

    if (!ensureResultsDirectory())
    {
        fprintf(stderr, "Could not create results folder: %s\n", RESULTS_DIRECTORY.c_str());
        return 1;
    }

    if (isBatch)
    {
        const std::size_t puzzleCount = argc >= 3 ? static_cast<std::size_t>(std::stoull(argv[2])) : 50;
        const std::vector<PuzzleRecord> puzzles =
            DatasetLoader::loadRandomPuzzles(DATASET_PATH, BATCH_MIN_CLUES, BATCH_MAX_CLUES, puzzleCount, seed);
        if (puzzles.empty())
        {
            fprintf(stderr, "Could not select batch puzzles from %s\n", DATASET_PATH.c_str());
            return 1;
        }

        std::uint64_t naiveNodes = 0;
        std::uint64_t bitmaskNodes = 0;
        std::uint64_t solver3Nodes = 0;
        std::uint64_t solver4Nodes = 0;
        std::uint64_t solver5Nodes = 0;
        double naiveTimeMs = 0.0;
        double bitmaskTimeMs = 0.0;
        double solver3TimeMs = 0.0;
        double solver4TimeMs = 0.0;
        double solver5TimeMs = 0.0;
        int unsolvedResults = 0;
        for (const PuzzleRecord& puzzle : puzzles)
        {
            // Batch mode measures solvers only.  It deliberately omits the
            // trace payload; normal mode captures one inspectable run per solver.
            const SolveResult naive = NaiveSolver::solve(puzzle.quiz, false);
            const SolveResult bitmask = BitmaskSolver::solve(puzzle.quiz, false);
            const SolveResult solver3 = Solver3::solve(puzzle.quiz, false);
            const SolveResult solver4 = Solver4::solve(puzzle.quiz, false);
            const SolveResult solver5 = Solver5::solve(puzzle.quiz, false);
            if (!naive.solved || !bitmask.solved || !solver3.solved || !solver4.solved || !solver5.solved)
            {
                unsolvedResults++;
            }
            naiveNodes += naive.nodesExplored;
            bitmaskNodes += bitmask.nodesExplored;
            solver3Nodes += solver3.nodesExplored;
            solver4Nodes += solver4.nodesExplored;
            solver5Nodes += solver5.nodesExplored;
            naiveTimeMs += benchmarkMedian<NaiveSolver>(puzzle.quiz);
            bitmaskTimeMs += benchmarkMedian<BitmaskSolver>(puzzle.quiz);
            solver3TimeMs += benchmarkMedian<Solver3>(puzzle.quiz);
            solver4TimeMs += benchmarkMedian<Solver4>(puzzle.quiz);
            solver5TimeMs += benchmarkMedian<Solver5>(puzzle.quiz);
        }

        const std::string reportPath = RESULTS_DIRECTORY + "\\batch_seed_" + std::to_string(seed) + ".txt";
        std::ofstream report(reportPath);
        report << std::fixed << std::setprecision(6)
               << "Batch puzzles: " << puzzles.size() << "\nSeed: " << seed << "\n"
               << "Naive: " << naiveNodes << " total nodes, " << naiveTimeMs / puzzles.size() << " ms average\n"
               << "Bitmask: " << bitmaskNodes << " total nodes, " << bitmaskTimeMs / puzzles.size() << " ms average\n"
               << "Solver3: " << solver3Nodes << " total nodes, " << solver3TimeMs / puzzles.size() << " ms average\n"
               << "Solver4: " << solver4Nodes << " total nodes, " << solver4TimeMs / puzzles.size() << " ms average\n"
               << "Solver5: " << solver5Nodes << " total nodes, " << solver5TimeMs / puzzles.size() << " ms average\n"
               << "Unsolved solver results: " << unsolvedResults << "\n";
        printf("Batch of %zu random puzzles\nNaive: %llu nodes, %.6f ms average\nBitmask: %llu nodes, %.6f ms average\nSolver3: %llu nodes, %.6f ms average\nSolver4: %llu nodes, %.6f ms average\nSolver5: %llu nodes, %.6f ms average\nReport: %s\n",
               puzzles.size(), (unsigned long long)naiveNodes, naiveTimeMs / puzzles.size(),
               (unsigned long long)bitmaskNodes, bitmaskTimeMs / puzzles.size(),
               (unsigned long long)solver3Nodes, solver3TimeMs / puzzles.size(),
               (unsigned long long)solver4Nodes, solver4TimeMs / puzzles.size(),
               (unsigned long long)solver5Nodes, solver5TimeMs / puzzles.size(), reportPath.c_str());
        if (unsolvedResults != 0)
        {
            fprintf(stderr, "Warning: at least one solver did not solve %d batch puzzle(s).\n", unsolvedResults);
            return 1;
        }
        return 0;
    }

    PuzzleRecord puzzle;
    printf("Selecting a random puzzle...\n");
    if (!DatasetLoader::loadRandomPuzzle(DATASET_PATH, VISUAL_MIN_CLUES, VISUAL_MAX_CLUES, seed, puzzle))
    {
        fprintf(stderr, "Could not select a puzzle from %s\n", DATASET_PATH.c_str());
        return 1;
    }

    printf("Random puzzle #%zu (seed %u, %d clues):\n", puzzle.datasetRow, seed, puzzle.clueCount);
    puzzle.quiz.print();

    SolveResult naiveResult = NaiveSolver::solve(puzzle.quiz);
    SolveResult bitmaskResult = BitmaskSolver::solve(puzzle.quiz);
    SolveResult solver3Result = Solver3::solve(puzzle.quiz);
    SolveResult solver4Result = Solver4::solve(puzzle.quiz);
    SolveResult solver5Result = Solver5::solve(puzzle.quiz);
    if (!naiveResult.solved || !bitmaskResult.solved || !solver3Result.solved ||
        !solver4Result.solved || !solver5Result.solved)
    {
        fprintf(stderr, "At least one solver could not solve the selected puzzle.\n");
        return 1;
    }
    naiveResult.timeMs = benchmarkMedian<NaiveSolver>(puzzle.quiz);
    bitmaskResult.timeMs = benchmarkMedian<BitmaskSolver>(puzzle.quiz);
    solver3Result.timeMs = benchmarkMedian<Solver3>(puzzle.quiz);
    solver4Result.timeMs = benchmarkMedian<Solver4>(puzzle.quiz);
    solver5Result.timeMs = benchmarkMedian<Solver5>(puzzle.quiz);

    const std::string name = "puzzle_" + std::to_string(puzzle.datasetRow) + "_seed_" + std::to_string(seed);
    const std::string reportPath = RESULTS_DIRECTORY + "\\" + name + ".txt";

    const std::string traceDirectory = RESULTS_DIRECTORY + "\\traces";
    if (!ensureDirectory(traceDirectory))
    {
        fprintf(stderr, "Could not create trace output folder: %s\n", traceDirectory.c_str());
        return 1;
    }
    const std::array<SolveResult, 5> traceResults = {naiveResult, bitmaskResult, solver3Result, solver4Result, solver5Result};
    const std::array<std::string, 5> solverNames = {"solver1", "solver2", "solver3", "solver4", "solver5"};
    const std::array<std::string, 5> traceFilenames = {
        name + "_solver1.html", name + "_solver2.html", name + "_solver3.html", name + "_solver4.html", name + "_solver5.html"};
    const auto writeTrace = [&](const SolveResult& result, const std::string& solverName) {
        return TreeTraversalVisualizer::writeHtml(result, "Puzzle #" + std::to_string(puzzle.datasetRow) + " · " + solverName,
                                                  traceDirectory + "\\" + name + "_" + solverName + ".html");
    };
    if (!writeTrace(naiveResult, "solver1") || !writeTrace(bitmaskResult, "solver2") ||
        !writeTrace(solver3Result, "solver3") || !writeTrace(solver4Result, "solver4") ||
        !writeTrace(solver5Result, "solver5"))
    {
        fprintf(stderr, "Could not create one or more trace visualizations.\n");
        return 1;
    }
    const std::string comparisonPath = traceDirectory + "\\" + name + "_comparison.html";
    if (!TreeTraversalVisualizer::writeComparisonHtml(traceResults, solverNames, traceFilenames,
                                                       "Puzzle #" + std::to_string(puzzle.datasetRow), comparisonPath))
    {
        fprintf(stderr, "Could not create the solver comparison visualization.\n");
        return 1;
    }

    printf("\nCompleted puzzle:\n");
    naiveResult.board.print();
    printf("\nNaive:   %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)naiveResult.nodesExplored, naiveResult.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Bitmask: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)bitmaskResult.nodesExplored, bitmaskResult.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Solver3: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)solver3Result.nodesExplored, solver3Result.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Solver4: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)solver4Result.nodesExplored, solver4Result.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Solver5: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)solver5Result.nodesExplored, solver5Result.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Trace visualizations: %s\n", traceDirectory.c_str());
    printf("Comparison visualization: %s\n", comparisonPath.c_str());
    printf("Node counts match: %s\n", naiveResult.nodesExplored == bitmaskResult.nodesExplored ? "yes" : "NO");
    printResearchTable(traceResults);

    std::ofstream report(reportPath);
    report << std::fixed << std::setprecision(6)
           << "Puzzle #" << puzzle.datasetRow << "\n"
           << "Seed: " << seed << "\n"
           << "Clues: " << puzzle.clueCount << "\n"
           << "Puzzle: " << puzzle.quiz.toString() << "\n"
           << "Naive: " << naiveResult.nodesExplored << " nodes, " << naiveResult.timeMs << " ms\n"
           << "Bitmask: " << bitmaskResult.nodesExplored << " nodes, " << bitmaskResult.timeMs << " ms\n"
           << "Solver3: " << solver3Result.nodesExplored << " nodes, " << solver3Result.timeMs << " ms\n"
           << "Solver4: " << solver4Result.nodesExplored << " nodes, " << solver4Result.timeMs << " ms\n"
           << "Solver5: " << solver5Result.nodesExplored << " nodes, " << solver5Result.timeMs << " ms\n"
           << "Trace HTML folder: " << traceDirectory << "\n"
           << "Comparison HTML: " << comparisonPath << "\n";
    writeResearchTable(report, traceResults);

    return 0;
}
