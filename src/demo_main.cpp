#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"
#include "treeTraversalVisualizer.hpp"

#include <cerrno>
#include <array>
#include <chrono>
#include <cctype>
#include <direct.h>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <windows.h>

namespace
{
const std::string RESULTS_DIRECTORY = R"(C:\Users\malob\CLionProjects\sudokku\results)";
constexpr int BENCHMARK_WARMUP_RUNS = 10;
constexpr int BENCHMARK_TIMED_RUNS = 100;
constexpr double BENCHMARK_TARGET_MS_PER_SOLVER = 1200.0;
volatile std::uint64_t timingGuard = 0;

void printUsage(const char* executable)
{
    std::cout
            << "Single-solution Sudoku demo\n\n"
            << "Usage:\n"
            << "  " << executable << "                         Enter a puzzle interactively\n"
            << "  " << executable << " --puzzle <81 cells> --trace-solver <1-8> [--benchmark-mode]\n\n"
            << "Use digits 1-9 for clues and 0 or . for blanks. Spaces, lines, |, +, and - are allowed,\n"
            << "so a normal 9-row Sudoku layout can be pasted directly. Every solver runs once; Solver 5 is the default trace.\n";
}

bool extractPuzzleCells(const std::string& input, std::string& cells, std::string& error)
{
    cells.clear();
    for (unsigned char character : input)
    {
        if (character >= '1' && character <= '9') cells += static_cast<char>(character);
        else if (character == '0' || character == '.') cells += '0';
        else if (std::isspace(character) || character == '|' || character == '+' || character == '-')
        {
            continue;
        }
        else
        {
            error = std::string("Unsupported character in puzzle input: '") +
                    static_cast<char>(character) + "'.";
            return false;
        }
    }
    return true;
}

bool parsePuzzle(const std::string& input, Board& board, std::string& error)
{
    std::string cells;
    if (!extractPuzzleCells(input, cells, error)) return false;
    if (cells.size() != 81)
    {
        error = "A Sudoku puzzle must contain exactly 81 cells; received " + std::to_string(cells.size()) + ".";
        return false;
    }

    Board checkedBoard;
    for (int index = 0; index < 81; ++index)
    {
        const int digit = cells[index] - '0';
        if (digit == 0) continue;

        const int row = index / 9;
        const int col = index % 9;
        if (!checkedBoard.isValidPlacement(row, col, digit))
        {
            error = "Conflicting given " + std::to_string(digit) + " at row " + std::to_string(row + 1) +
                    ", column " + std::to_string(col + 1) + ".";
            return false;
        }
        checkedBoard.cells[row][col] = digit;
    }

    board = Board::fromString(cells);
    return true;
}

bool readPuzzleInteractively(Board& board)
{
    std::cout << "Paste one 81-cell puzzle or nine rows. Use 0 or . for blanks, then press Enter.\n";
    std::string input;
    std::string normalized;
    std::string error;
    while (normalized.size() < 81 && std::cin.good())
    {
        std::string line;
        if (!std::getline(std::cin, line)) break;
        input += line;
        input += '\n';
        if (!extractPuzzleCells(input, normalized, error))
        {
            std::cerr << error << '\n';
            return false;
        }
        if (normalized.size() > 81)
        {
            std::cerr << "A Sudoku puzzle cannot contain more than 81 cells.\n";
            return false;
        }
    }

    if (!parsePuzzle(input, board, error))
    {
        std::cerr << error << '\n';
        return false;
    }
    return true;
}

bool parseSolverNumber(const std::string& value, int& solver)
{
    if (value.size() != 1 || value[0] < '1' || value[0] > '8') return false;
    solver = value[0] - '0';
    return true;
}

const char* solverName(int solver)
{
    switch (solver)
    {
    case 1:
        return "Solver 1 — naive array scan";
    case 2:
        return "Solver 2 — bitmask board";
    case 3:
        return "Solver 3 — bitmask candidates + singles/pointing pairs";
    case 4:
        return "Solver 4 — bitmask candidates + naked pairs";
    case 5:
        return "Solver 5 — bitmask candidates + X-Wing";
    case 6:
        return "Array Solver 3 — array candidates + singles/pointing pairs";
    case 7:
        return "Array Solver 4 — array candidates + naked pairs";
    case 8:
        return "Array Solver 5 — array candidates + X-Wing";
    default:
        return "Unknown solver";
    }
}

SolveResult solveOnce(const Board& board, int solver, bool captureTrace = true)
{
    switch (solver)
    {
    case 1:
        return NaiveSolver::solve(board, captureTrace);
    case 2:
        return BitmaskSolver::solve(board, captureTrace);
    case 3:
        return Solver3::solve(board, captureTrace);
    case 4:
        return Solver4::solve(board, captureTrace);
    case 5:
        return Solver5::solve(board, captureTrace);
    case 6:
        return ArraySolver3::solve(board, captureTrace);
    case 7:
        return ArraySolver4::solve(board, captureTrace);
    case 8:
        return ArraySolver5::solve(board, captureTrace);
    default:
        return {};
    }
}

struct BenchmarkResult
{
    double medianMs = 0.0;
    std::size_t timedRuns = 0;
};

BenchmarkResult benchmarkMedian(const Board& board, int solver)
{
    const auto calibrationStart = std::chrono::steady_clock::now();
    timingGuard ^= solveOnce(board, solver, false).nodesExplored;
    const auto calibrationEnd = std::chrono::steady_clock::now();
    const double calibrationMs = std::chrono::duration<double, std::milli>(calibrationEnd - calibrationStart).count();
    const std::size_t timedRuns = std::max<std::size_t>(1, std::min<std::size_t>(
        BENCHMARK_TIMED_RUNS, static_cast<std::size_t>(BENCHMARK_TARGET_MS_PER_SOLVER / std::max(calibrationMs, 0.001))));
    const std::size_t extraWarmups = std::min<std::size_t>(BENCHMARK_WARMUP_RUNS - 1, timedRuns - 1);
    for (std::size_t run = 0; run < extraWarmups; ++run)
    {
        timingGuard ^= solveOnce(board, solver, false).nodesExplored;
    }

    std::vector<double> samples;
    samples.reserve(timedRuns);
    for (std::size_t run = 0; run < timedRuns; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        const SolveResult result = solveOnce(board, solver, false);
        const auto end = std::chrono::steady_clock::now();
        timingGuard ^= result.nodesExplored;
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(samples.begin(), samples.end());
    return {samples[samples.size() / 2], timedRuns};
}

void enableBenchmarkMode()
{
    // Optional only: keeps routine demonstrations responsive while allowing
    // a user to reduce scheduler migration/noise for a dedicated benchmark.
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
    {
        std::cerr << "Warning: could not raise process priority for benchmark mode.\n";
    }

    DWORD_PTR processMask = 0;
    DWORD_PTR systemMask = 0;
    if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask) && processMask != 0)
    {
        const DWORD_PTR firstAvailableCore = processMask & (~processMask + 1);
        if (SetThreadAffinityMask(GetCurrentThread(), firstAvailableCore) == 0)
        {
            std::cerr << "Warning: could not pin the benchmark thread to one CPU core.\n";
        }
    }
}

bool ensureDirectory(const std::string& path)
{
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

struct ResultSummary
{
    int maxDepth = 0;
    std::size_t guesses = 0;
    std::size_t deadEnds = 0;
    std::size_t placements = 0;
    std::size_t eliminations = 0;
};

ResultSummary summarize(const SolveResult& result)
{
    ResultSummary summary;
    for (const TraceEvent& event : result.trace)
    {
        summary.maxDepth = std::max(summary.maxDepth, event.depth);
        summary.guesses += event.isGuess;
        summary.deadEnds += event.isGuess && event.leadsToConflict;
        summary.placements += event.placement.has_value();
        summary.eliminations += event.eliminations.size();
    }
    return summary;
}

void printComparison(const std::array<SolveResult, 8>& results, const std::array<std::size_t, 8>& timedRuns)
{
    static const std::array<const char*, 8> names =
    {
        "Naive", "Bitmask", "Solver3 bitmask", "Solver4 bitmask", "Solver5 bitmask",
        "Solver3 array", "Solver4 array", "Solver5 array"
    };
    std::cout << "\nBenchmark comparison (median of trace-free runs after warm-ups; up to " << BENCHMARK_TIMED_RUNS << " samples):\n"
              << std::left << std::setw(18) << "Solver" << std::right << std::setw(12) << "Time (ms)"
              << std::setw(9) << "Samples" << std::setw(12) << "Max depth" << std::setw(11) << "Guesses" << std::setw(12) << "Dead ends"
              << std::setw(12) << "Placements" << std::setw(14) << "Eliminations" << std::setw(10) << "Events" << '\n';
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const ResultSummary summary = summarize(results[index]);
        std::cout << std::left << std::setw(18) << names[index] << std::right << std::fixed << std::setprecision(4)
                  << std::setw(12) << results[index].timeMs << std::setw(9) << timedRuns[index] << std::setw(12) << summary.maxDepth
                  << std::setw(11) << summary.guesses << std::setw(12) << summary.deadEnds << std::setw(12) << summary.placements
                  << std::setw(14) << summary.eliminations << std::setw(10) << results[index].trace.size() << '\n';
    }
    std::cout << "Slow solvers receive fewer samples to keep one puzzle responsive; Max depth = deepest search branch.\n";
}
}

int main(int argc, char* argv[])
{
    int traceSolver = 5;
    bool benchmarkMode = false;
    std::string puzzleText;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--help" || argument == "-h")
        {
            printUsage(argv[0]);
            return 0;
        }
        if (argument == "--puzzle")
        {
            if (++index >= argc)
            {
                std::cerr << "--puzzle needs an 81-cell value.\n";
                return 1;
            }
            puzzleText = argv[index];
            continue;
        }
        if (argument == "--trace-solver" || argument == "--solver")
        {
            if (++index >= argc || !parseSolverNumber(argv[index], traceSolver))
            {
                std::cerr << "--trace-solver must be a number from 1 through 8.\n";
                return 1;
            }
            continue;
        }
        if (argument == "--benchmark-mode")
        {
            benchmarkMode = true;
            continue;
        }

        std::cerr << "Unknown option: " << argument << "\n";
        printUsage(argv[0]);
        return 1;
    }

    Board puzzle;
    if (puzzleText.empty())
    {
        if (!readPuzzleInteractively(puzzle)) return 1;
    }
    else
    {
        std::string error;
        if (!parsePuzzle(puzzleText, puzzle, error))
        {
            std::cerr << error << '\n';
            return 1;
        }
    }

    if (benchmarkMode)
    {
        enableBenchmarkMode();
        std::cout << "Benchmark mode: high process priority and one-core affinity requested.\n";
    }

    if (!ensureDirectory(RESULTS_DIRECTORY))
    {
        std::cerr << "Could not create results folder: " << RESULTS_DIRECTORY << '\n';
        return 1;
    }
    const std::string demoDirectory = RESULTS_DIRECTORY + "\\demo";
    if (!ensureDirectory(demoDirectory))
    {
        std::cerr << "Could not create demo output folder: " << demoDirectory << '\n';
        return 1;
    }

    std::cout << "\nCapturing one trace per solver and benchmarking each solver. Starting puzzle:\n";
    puzzle.print();
    std::array<SolveResult, 8> results;
    std::array<std::size_t, 8> timedRuns{};
    for (int solver = 1; solver <= 8; ++solver)
    {
        results[solver - 1] = solveOnce(puzzle, solver);
        const BenchmarkResult benchmark = benchmarkMedian(puzzle, solver);
        results[solver - 1].timeMs = benchmark.medianMs;
        timedRuns[solver - 1] = benchmark.timedRuns;
    }
    printComparison(results, timedRuns);
    const SolveResult& tracedResult = results[traceSolver - 1];

    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch()).count();
    std::array<std::string, 5> traceFiles{};
    for (int solver = 1; solver <= 5; ++solver)
    {
        const std::string baseName = "manual_solver" + std::to_string(solver) + "_" + std::to_string(timestamp);
        const std::string solutionPath = demoDirectory + "\\" + baseName + "_solution_trace.html";
        const std::string runtimePath = demoDirectory + "\\" + baseName + "_full_runtime_trace.html";
        const std::string title = std::string("Manual puzzle · ") + solverName(solver);
        if (!TreeTraversalVisualizer::writeHtml(results[solver - 1], title, solutionPath) ||
                !TreeTraversalVisualizer::writeRuntimeHtml(results[solver - 1], title, runtimePath))
        {
            std::cerr << "Could not write one or more trace files.\n";
            return 1;
        }
        traceFiles[solver - 1] = baseName + "_solution_trace.html";
    }
    const std::string comparisonPath = demoDirectory + "\\manual_solvers_1_to_5_" + std::to_string(timestamp) + "_comparison.html";
    const std::array<SolveResult, 5> standardResults = {results[0], results[1], results[2], results[3], results[4]};
    const std::array<std::string, 5> standardNames = {"solver1", "solver2", "solver3", "solver4", "solver5"};
    if (!TreeTraversalVisualizer::writeComparisonHtml(standardResults, standardNames, traceFiles,
            "Manual puzzle · Solvers 1-5", comparisonPath))
    {
        std::cerr << "Could not write the Solvers 1-5 comparison trace.\n";
        return 1;
    }

    if (tracedResult.solved)
    {
        std::cout << "\nFirst solution found:\n";
        tracedResult.board.print();
    }
    else
    {
        std::cout << "\nNo solution was found for this puzzle. The runtime trace was still written.\n";
    }

    bool allSolved = true;
    for (const SolveResult& result : results) allSolved = allSolved && result.solved;
    std::cout << "\nDetailed trace solver shown above: " << solverName(traceSolver) << "\n"
              << "Solvers 1-5 solution and full-runtime traces: " << demoDirectory << "\n"
              << "Solvers 1-5 comparison: " << comparisonPath << "\n";
    return allSolved ? 0 : 2;
}
