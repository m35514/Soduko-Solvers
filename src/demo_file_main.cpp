#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <windows.h>

namespace
{
const std::filesystem::path PROJECT_ROOT = R"(C:\Users\malob\CLionProjects\sudokku)";
constexpr int BENCHMARK_WARMUP_RUNS = 10;
constexpr int BENCHMARK_TIMED_RUNS = 100;
constexpr double BENCHMARK_TARGET_MS_PER_SOLVER = 1200.0;
volatile std::uint64_t timingGuard = 0;
const std::array<const char*, 8> SOLVER_NAMES = {
    "Naive", "Bitmask", "Solver3 bitmask", "Solver4 bitmask", "Solver5 bitmask",
    "Solver3 array", "Solver4 array", "Solver5 array"
};

struct Options
{
    std::filesystem::path inputPath = PROJECT_ROOT / "src/data/sudokuwiki_examples.txt";
    std::filesystem::path outputPath = PROJECT_ROOT / "results/sudokuwiki_examples_demo.csv";
    bool benchmarkMode = false;
};

struct NamedPuzzle
{
    std::string puzzleName;
    Board board;
};

// Report elapsed time without reading counters while a solver is updating them.
class SolveProgress
{
public:
    explicit SolveProgress(std::string solverLabel) : label(std::move(solverLabel))
    {
        std::cout << this->label << " - Collecting statistics...\n" << std::flush;
        reporter = std::thread([this]
        {
            std::unique_lock<std::mutex> lock(mutex);
            while (!changed.wait_for(lock, std::chrono::seconds(10), [this] { return finished; }))
            {
                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - started).count();
                std::cout << label << " - Still " << stage << " (" << elapsed << " seconds elapsed).\n" << std::flush;
            }
        });
    }

    void benchmarking(std::uint64_t events)
    {
        std::lock_guard<std::mutex> lock(mutex);
        stage = "benchmarking";
        std::cout << label << " - Statistics ready (" << events << " events); benchmarking...\n" << std::flush;
    }

    ~SolveProgress()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            finished = true;
        }
        changed.notify_one();
        if (reporter.joinable()) reporter.join();
    }

    SolveProgress(const SolveProgress&) = delete;
    SolveProgress& operator=(const SolveProgress&) = delete;

private:
    std::string label;
    std::string stage = "collecting statistics";
    const std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
    std::mutex mutex;
    std::condition_variable changed;
    bool finished = false;
    std::thread reporter;
};

void printUsage(const char* executable)
{
    std::cout << "File-based Sudoku demo\n\nUsage: " << executable
              << " [--input FILE] [--output FILE] [--benchmark-mode]\n"
              << "Input: a puzzle name followed by one 81-cell puzzle line; blank lines are optional.\n"
              << "Use 1-9 for clues and 0 or . for blanks; spaces, |, +, and - are allowed.\n"
              << "Runs all eight solvers for every puzzle and writes a CSV, with no trace files.\n"
              << "Default input: " << Options{}.inputPath.string() << '\n'
              << "Default output: " << Options{}.outputPath.string() << '\n';
}

Options parseOptions(int argc, char* argv[])
{
    Options options;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--benchmark-mode") options.benchmarkMode = true;
        else if (argument == "--input" || argument == "--output")
        {
            if (++index >= argc || argv[index][0] == '\0' || std::string(argv[index]).rfind("--", 0) == 0)
                throw std::runtime_error("Missing file path after " + argument);
            if (argument == "--input") options.inputPath = argv[index];
            else options.outputPath = argv[index];
        }
        else throw std::runtime_error("Unknown option: " + argument);
    }
    return options;
}

bool extractPuzzleCells(const std::string& input, std::string& cells, std::string& error)
{
    cells.clear();
    for (unsigned char character : input)
    {
        if (character >= '1' && character <= '9') cells += static_cast<char>(character);
        else if (character == '0' || character == '.') cells += '0';
        else if (std::isspace(character) || character == '|' || character == '+' || character == '-') continue;
        else
        {
            error = std::string("Unsupported character in puzzle input: '") + static_cast<char>(character) + "'.";
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

std::string trim(const std::string& line)
{
    const auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char ch) { return std::isspace(ch); });
    const auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    return first < last ? std::string(first, last) : std::string();
}

std::vector<NamedPuzzle> readPuzzles(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open input file: " + path.string());
    std::vector<NamedPuzzle> puzzles;
    std::string puzzleName;
    std::size_t nameLine = 0;
    std::size_t lineNumber = 0;
    std::string line;
    while (std::getline(input, line))
    {
        ++lineNumber;
        if (lineNumber == 1 && line.compare(0, 3, "\xef\xbb\xbf") == 0) line.erase(0, 3);
        line = trim(line);
        if (line.empty()) continue;
        if (puzzleName.empty())
        {
            puzzleName = line;
            nameLine = lineNumber;
            continue;
        }
        Board board;
        std::string error;
        if (!parsePuzzle(line, board, error))
            throw std::runtime_error(path.string() + ": line " + std::to_string(lineNumber) +
                                     ", puzzle '" + puzzleName + "': " + error);
        puzzles.push_back({puzzleName, board});
        puzzleName.clear();
    }
    if (input.bad()) throw std::runtime_error("Cannot read input file: " + path.string());
    if (!puzzleName.empty())
        throw std::runtime_error(path.string() + ": line " + std::to_string(nameLine) +
                                 ", puzzle '" + puzzleName + "': missing puzzle line.");
    if (puzzles.empty()) throw std::runtime_error("Input file contains no puzzles: " + path.string());
    return puzzles;
}

SolveResult solveOnce(const Board& board, int solver, bool captureTrace = false, SolveStatistics* statistics = nullptr)
{
    switch (solver)
    {
    case 1: return NaiveSolver::solve(board, captureTrace, statistics);
    case 2: return BitmaskSolver::solve(board, captureTrace, statistics);
    case 3: return Solver3::solve(board, captureTrace, statistics);
    case 4: return Solver4::solve(board, captureTrace, statistics);
    case 5: return Solver5::solve(board, captureTrace, statistics);
    case 6: return ArraySolver3::solve(board, captureTrace, statistics);
    case 7: return ArraySolver4::solve(board, captureTrace, statistics);
    case 8: return ArraySolver5::solve(board, captureTrace, statistics);
    default: return {};
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
        timingGuard ^= solveOnce(board, solver, false).nodesExplored;
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
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        std::cerr << "Warning: could not raise process priority for benchmark mode.\n";
    DWORD_PTR processMask = 0;
    DWORD_PTR systemMask = 0;
    if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask) && processMask != 0)
    {
        const DWORD_PTR firstAvailableCore = processMask & (~processMask + 1);
        if (SetThreadAffinityMask(GetCurrentThread(), firstAvailableCore) == 0)
            std::cerr << "Warning: could not pin the benchmark thread to one CPU core.\n";
    }
}

std::string csv(const std::string& value)
{
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string escaped = "\"";
    for (char character : value)
    {
        if (character == '"') escaped += '"';
        escaped += character;
    }
    return escaped + '"';
}

void printComparison(const std::string& puzzleName, const std::array<BenchmarkResult, 8>& benchmarks,
                     const std::array<SolveStatistics, 8>& summaries)
{
    std::cout << '\n' << puzzleName << " - Benchmark comparison (median of trace-free runs after warm-ups; up to "
              << BENCHMARK_TIMED_RUNS << " samples):\n"
              << std::left << std::setw(18) << "Solver" << std::right << std::setw(12) << "Time (ms)"
              << std::setw(9) << "Samples" << std::setw(12) << "Max depth" << std::setw(11) << "Guesses"
              << std::setw(12) << "Dead ends" << std::setw(12) << "Placements"
              << std::setw(14) << "Eliminations" << std::setw(10) << "Events" << '\n';
    for (std::size_t index = 0; index < summaries.size(); ++index)
    {
        const SolveStatistics& summary = summaries[index];
        std::cout << std::left << std::setw(18) << SOLVER_NAMES[index] << std::right << std::fixed << std::setprecision(4)
                  << std::setw(12) << benchmarks[index].medianMs << std::setw(9) << benchmarks[index].timedRuns
                  << std::setw(12) << summary.maxDepth << std::setw(11) << summary.guesses
                  << std::setw(12) << summary.deadEnds << std::setw(12) << summary.placements
                  << std::setw(14) << summary.eliminations << std::setw(10) << summary.events << '\n';
    }
    std::cout << "Slow solvers receive fewer samples to keep one puzzle responsive; Max depth = deepest search branch.\n";
}

int run(const Options& options)
{
    // Validate every record before opening (and potentially overwriting) the CSV.
    const std::vector<NamedPuzzle> puzzles = readPuzzles(options.inputPath);
    if (std::filesystem::exists(options.outputPath) && std::filesystem::equivalent(options.inputPath, options.outputPath))
        throw std::runtime_error("Input and output must be different files.");
    if (options.outputPath.has_parent_path()) std::filesystem::create_directories(options.outputPath.parent_path());
    std::ofstream output(options.outputPath, std::ios::trunc);
    if (!output) throw std::runtime_error("Cannot write CSV: " + options.outputPath.string());
    output << "Puzzle Name,Puzzle String,Solver,Solved,Solution String,Time (ms),Samples,Max depth,"
              "Guesses,Dead ends,Placements,Eliminations,Events\n";
    if (!output) throw std::runtime_error("Cannot write CSV header: " + options.outputPath.string());
    if (options.benchmarkMode)
    {
        enableBenchmarkMode();
        std::cout << "Benchmark mode: high process priority and one-core affinity requested.\n";
    }
    bool allSolved = true;
    std::size_t completed = 0;
    for (const NamedPuzzle& puzzle : puzzles)
    {
        const std::string& puzzleName = puzzle.puzzleName;
        std::cout << '\n' << puzzleName << " - Starting puzzle (" << ++completed << '/' << puzzles.size() << "):\n";
        puzzle.board.print();
        std::cout << std::flush;
        std::array<BenchmarkResult, 8> benchmarks{};
        std::array<SolveStatistics, 8> summaries{};
        Board displayedSolution;
        bool displayedSolved = false;
        for (int solver = 1; solver <= 8; ++solver)
        {
            std::cout << puzzleName << " - Running " << SOLVER_NAMES[solver - 1] << "...\n" << std::flush;
            SolveProgress progress(puzzleName + " / " + SOLVER_NAMES[solver - 1]);
            SolveStatistics summary;
            const SolveResult result = solveOnce(puzzle.board, solver, false, &summary);
            progress.benchmarking(summary.events);
            const BenchmarkResult benchmark = benchmarkMedian(puzzle.board, solver);
            summaries[solver - 1] = summary;
            benchmarks[solver - 1] = benchmark;
            allSolved = allSolved && result.solved;
            if (solver == 5)
            {
                displayedSolution = result.board;
                displayedSolved = result.solved;
            }
            output << csv(puzzleName) << ',' << puzzle.board.toString() << ',' << SOLVER_NAMES[solver - 1] << ','
                   << (result.solved ? "true" : "false") << ',' << (result.solved ? result.board.toString() : "") << ','
                   << std::fixed << std::setprecision(6) << benchmark.medianMs << ',' << benchmark.timedRuns << ','
                   << summary.maxDepth << ',' << summary.guesses << ',' << summary.deadEnds << ','
                   << summary.placements << ',' << summary.eliminations << ',' << summary.events << '\n';
            output.flush();
            if (!output) throw std::runtime_error("CSV write failed for puzzle '" + puzzleName + "'.");
        }
        printComparison(puzzleName, benchmarks, summaries);
        if (displayedSolved)
        {
            std::cout << '\n' << puzzleName << " - First solution found (Solver 5):\n";
            displayedSolution.print();
        }
        else std::cout << '\n' << puzzleName << " - No solution was found by Solver 5.\n";
        std::cout << std::flush;
    }
    output.close();
    if (!output) throw std::runtime_error("Cannot finish CSV: " + options.outputPath.string());
    std::cout << "\nWrote " << puzzles.size() * 8 << " solver rows to " << options.outputPath.string() << '\n';
    return allSolved ? 0 : 2;
}
}

int main(int argc, char* argv[])
{
    try
    {
        for (int index = 1; index < argc; ++index)
        {
            if (std::string(argv[index]) == "--help" || std::string(argv[index]) == "-h")
            {
                printUsage(argv[0]);
                return 0;
            }
        }
        return run(parseOptions(argc, argv));
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
