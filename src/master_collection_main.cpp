#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
// These collections give a repeatable mix of ordinary and technique-rich
// puzzles. The overlapping 17-clue collections are deliberately omitted.
const std::array<const char*, 10> COLLECTIONS = {
    "Sudoku_easy.txt",
    "Sudoku_medium.txt",
    "Sudoku_hard.txt",
    "Sudoku_diabolical.txt",
    "learningcurve_2500.txt",
    "No_One_Trick_Ponies_250.txt",
    "SUPER160.txt",
    "Almost Daily Nightmare_400.txt",
    "Tarek_Pearly6000.txt",
    "Top_50K_Toughest.txt"
};

const std::array<const char*, 8> SOLVER_NAMES = {
    "Naive", "Bitmask", "Solver3_bitmask", "Solver4_bitmask",
    "Solver5_bitmask", "Solver3_array", "Solver4_array", "Solver5_array"
};

volatile std::uint64_t timingGuard = 0;

struct Options
{
    std::string dataDirectory = "src\\data\\SUDOKU Master collection";
    std::string outputPath = "results\\master_collection_benchmark.csv";
    std::size_t sampleSize = 200;
    std::uint32_t seed = 20260928;
    DWORD timeoutMs = 15000;
    std::size_t maxTimedRuns = 31;
    double targetTimingMs = 250.0;
    std::string onlyCollection;
};

struct PuzzleSample
{
    std::string collection;
    std::size_t sourceLine = 0;
    std::string puzzle;
    int clues = 0;
};

struct SolverMetric
{
    std::string status = "error";
    double medianTimeMs = 0.0;
    std::size_t timedRuns = 0;
    int maxDepth = 0;
    std::size_t guesses = 0;
    std::size_t deadEnds = 0;
    std::size_t placements = 0;
    std::size_t eliminations = 0;
    std::size_t events = 0;
    bool timingReady = false;
    bool traceReady = false;
};

void printUsage(const char* executable)
{
    std::cout << "Usage: " << executable << " [--data-dir DIR] [--output FILE]"
              << " [--sample-size N] [--seed N] [--timeout-ms N]"
              << " [--max-timed-runs N] [--target-timing-ms N]"
              << " [--only FILENAME]\n"
              << "Samples each selected collection with a fixed seed and writes one CSV.\n"
              << "Time is the median of trace-free runs; the other measures come\n"
              << "from one in-memory trace per solver. No trace files are saved.\n";
}

std::uint64_t parseUnsigned(const std::string& text, const char* option)
{
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](unsigned char ch) { return std::isdigit(ch); }))
        throw std::runtime_error(std::string("Invalid ") + option + ": " + text);
    std::size_t used = 0;
    const std::uint64_t value = std::stoull(text, &used);
    if (used != text.size()) throw std::runtime_error(std::string("Invalid ") + option + ": " + text);
    return value;
}

Options parseOptions(int argc, char* argv[])
{
    Options options;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--help" || argument == "-h")
        {
            printUsage(argv[0]);
            std::exit(0);
        }
        if (index + 1 >= argc) throw std::runtime_error("Missing value after " + argument);
        const std::string value = argv[++index];
        if (argument == "--data-dir") options.dataDirectory = value;
        else if (argument == "--output") options.outputPath = value;
        else if (argument == "--sample-size") options.sampleSize = static_cast<std::size_t>(parseUnsigned(value, "sample size"));
        else if (argument == "--seed")
        {
            const std::uint64_t parsed = parseUnsigned(value, "seed");
            if (parsed > std::numeric_limits<std::uint32_t>::max()) throw std::runtime_error("Seed is too large");
            options.seed = static_cast<std::uint32_t>(parsed);
        }
        else if (argument == "--timeout-ms")
        {
            const std::uint64_t parsed = parseUnsigned(value, "timeout");
            if (parsed >= INFINITE) throw std::runtime_error("Timeout is too large");
            options.timeoutMs = static_cast<DWORD>(parsed);
        }
        else if (argument == "--max-timed-runs") options.maxTimedRuns = static_cast<std::size_t>(parseUnsigned(value, "timed runs"));
        else if (argument == "--target-timing-ms") options.targetTimingMs = static_cast<double>(parseUnsigned(value, "timing target"));
        else if (argument == "--only") options.onlyCollection = value;
        else throw std::runtime_error("Unknown option: " + argument);
    }
    if (options.sampleSize == 0) throw std::runtime_error("Sample size must be positive");
    if (options.timeoutMs == 0) throw std::runtime_error("Timeout must be positive");
    if (options.maxTimedRuns == 0) throw std::runtime_error("Timed run count must be positive");
    if (options.targetTimingMs <= 0) throw std::runtime_error("Timing target must be positive");
    return options;
}

std::uint32_t filenameHash(const std::string& name)
{
    std::uint32_t hash = 2166136261u;
    for (unsigned char character : name)
    {
        hash ^= character;
        hash *= 16777619u;
    }
    return hash;
}

std::string joinPath(const std::string& directory, const std::string& name)
{
    if (directory.empty()) return name;
    if (directory.back() == '\\' || directory.back() == '/') return directory + name;
    return directory + '\\' + name;
}

std::string filenameOf(const std::string& path)
{
    const std::size_t separator = path.find_last_of("\\/");
    return separator == std::string::npos ? path : path.substr(separator + 1);
}

bool directoryExists(const std::string& path)
{
    const DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void createDirectoryIfNeeded(const std::string& path)
{
    if (path.empty() || directoryExists(path)) return;
    if (!CreateDirectoryA(path.c_str(), nullptr) && !directoryExists(path))
        throw std::runtime_error("Cannot create output directory: " + path);
}

void createOutputParentDirectories(const std::string& outputPath)
{
    const std::size_t lastSeparator = outputPath.find_last_of("\\/");
    if (lastSeparator == std::string::npos || lastSeparator == 0) return;
    std::string parent = outputPath.substr(0, lastSeparator);
    if (lastSeparator == 2 && outputPath[1] == ':') parent = outputPath.substr(0, 3);
    for (std::size_t index = 0; index < parent.size(); ++index)
    {
        if (parent[index] != '\\' && parent[index] != '/') continue;
        if (index == 0 || (index == 2 && parent[1] == ':')) continue;
        createDirectoryIfNeeded(parent.substr(0, index));
    }
    createDirectoryIfNeeded(parent);
}

bool normalizePuzzle(std::string& line, int& clues)
{
    const auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char ch) { return std::isspace(ch); });
    const auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    line = first < last ? std::string(first, last) : std::string();
    if (line.size() != 81) return false;

    clues = 0;
    for (char& character : line)
    {
        if (character == '.') character = '0';
        else if (character < '0' || character > '9') return false;
        if (character != '0') ++clues;
    }

    // Reject contradictory givens before passing a grid to the old solvers.
    std::array<int, 9> rowMask{};
    std::array<int, 9> colMask{};
    std::array<int, 9> boxMask{};
    for (int cell = 0; cell < 81; ++cell)
    {
        const int digit = line[cell] - '0';
        if (digit == 0) continue;
        const int row = cell / 9;
        const int col = cell % 9;
        const int box = (row / 3) * 3 + col / 3;
        const int bit = 1 << digit;
        if ((rowMask[row] | colMask[col] | boxMask[box]) & bit) return false;
        rowMask[row] |= bit;
        colMask[col] |= bit;
        boxMask[box] |= bit;
    }
    return true;
}

bool validCompletedBoard(const Board& result, const Board& startingBoard)
{
    constexpr int allDigits = 0x3fe;
    for (int row = 0; row < 9; ++row)
    {
        int rowMask = 0;
        int colMask = 0;
        for (int col = 0; col < 9; ++col)
        {
            const int rowDigit = result.cells[row][col];
            const int colDigit = result.cells[col][row];
            if (rowDigit < 1 || rowDigit > 9 || colDigit < 1 || colDigit > 9) return false;
            if (startingBoard.cells[row][col] != 0 && startingBoard.cells[row][col] != rowDigit) return false;
            rowMask |= 1 << rowDigit;
            colMask |= 1 << colDigit;
        }
        if (rowMask != allDigits || colMask != allDigits) return false;
    }
    for (int boxRow = 0; boxRow < 9; boxRow += 3)
    {
        for (int boxCol = 0; boxCol < 9; boxCol += 3)
        {
            int mask = 0;
            for (int row = boxRow; row < boxRow + 3; ++row)
                for (int col = boxCol; col < boxCol + 3; ++col)
                    mask |= 1 << result.cells[row][col];
            if (mask != allDigits) return false;
        }
    }
    return true;
}

std::vector<PuzzleSample> sampleCollection(const std::string& path, std::size_t count, std::uint32_t seed)
{
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open dataset: " + path);

    const std::string collection = filenameOf(path);
    std::mt19937 generator(seed ^ filenameHash(collection));
    std::vector<PuzzleSample> selected;
    selected.reserve(count);
    std::size_t eligible = 0;
    std::size_t sourceLine = 0;
    std::size_t skipped = 0;
    std::string line;
    while (std::getline(input, line))
    {
        ++sourceLine;
        int clues = 0;
        if (!normalizePuzzle(line, clues))
        {
            ++skipped;
            continue;
        }
        ++eligible;
        PuzzleSample sample{collection, sourceLine, line, clues};
        if (selected.size() < count) selected.push_back(std::move(sample));
        else
        {
            std::uniform_int_distribution<std::size_t> choose(0, eligible - 1);
            const std::size_t replacement = choose(generator);
            if (replacement < count) selected[replacement] = std::move(sample);
        }
    }
    std::sort(selected.begin(), selected.end(), [](const PuzzleSample& left, const PuzzleSample& right) {
        return left.sourceLine < right.sourceLine;
    });
    std::cout << collection << ": " << eligible << " valid puzzles, " << skipped
              << " skipped lines, " << selected.size() << " selected\n";
    return selected;
}

SolveResult solveOnce(const Board& board, int solver, bool captureTrace)
{
    switch (solver)
    {
    case 1: return NaiveSolver::solve(board, captureTrace);
    case 2: return BitmaskSolver::solve(board, captureTrace);
    case 3: return Solver3::solve(board, captureTrace);
    case 4: return Solver4::solve(board, captureTrace);
    case 5: return Solver5::solve(board, captureTrace);
    case 6: return ArraySolver3::solve(board, captureTrace);
    case 7: return ArraySolver4::solve(board, captureTrace);
    case 8: return ArraySolver5::solve(board, captureTrace);
    default: throw std::runtime_error("Invalid solver number");
    }
}

SolverMetric summarizeTrace(const SolveResult& result)
{
    SolverMetric metric;
    for (const TraceEvent& event : result.trace)
    {
        metric.maxDepth = std::max(metric.maxDepth, event.depth);
        metric.guesses += event.isGuess;
        metric.deadEnds += event.isGuess && event.leadsToConflict;
        metric.placements += event.placement.has_value();
        metric.eliminations += event.eliminations.size();
    }
    metric.events = result.trace.size();
    metric.traceReady = true;
    return metric;
}

int workerMain(int solver, std::string puzzle, std::size_t maxTimedRuns, double targetTimingMs)
{
    int clues = 0;
    if (!normalizePuzzle(puzzle, clues)) return 2;
    const Board board = Board::fromString(puzzle);
    const auto calibrationStart = std::chrono::steady_clock::now();
    const SolveResult first = solveOnce(board, solver, false);
    const auto calibrationEnd = std::chrono::steady_clock::now();
    if (first.solved && !validCompletedBoard(first.board, board))
        throw std::runtime_error("Solver returned an invalid completed board");
    timingGuard ^= first.nodesExplored;
    const double calibrationMs = std::chrono::duration<double, std::milli>(
        calibrationEnd - calibrationStart).count();
    // Preserve one measured solve even if a later repetition or trace times out.
    std::cout << "TIMING\t" << std::fixed << std::setprecision(6)
              << calibrationMs << "\t1" << std::endl;
    const double desiredRuns = targetTimingMs / std::max(calibrationMs, 0.001);
    const std::size_t targetRuns = desiredRuns >= static_cast<double>(maxTimedRuns)
        ? maxTimedRuns : std::max<std::size_t>(1, static_cast<std::size_t>(desiredRuns));
    const std::size_t warmups = std::min<std::size_t>(9, targetRuns - 1);
    for (std::size_t run = 0; run < warmups; ++run)
    {
        const SolveResult warmup = solveOnce(board, solver, false);
        timingGuard ^= warmup.nodesExplored;
    }
    std::vector<double> samples;
    samples.reserve(targetRuns);
    for (std::size_t run = 0; run < targetRuns; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        const SolveResult result = solveOnce(board, solver, false);
        const auto end = std::chrono::steady_clock::now();
        timingGuard ^= result.nodesExplored;
        if (result.solved != first.solved || result.nodesExplored != first.nodesExplored ||
            result.board.toString() != first.board.toString())
            throw std::runtime_error("Timed runs were inconsistent");
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(samples.begin(), samples.end());
    std::cout << "TIMING\t" << std::fixed << std::setprecision(6)
              << samples[samples.size() / 2] << '\t' << samples.size() << std::endl;

    const SolveResult traced = solveOnce(board, solver, true);
    if (traced.solved != first.solved || traced.nodesExplored != first.nodesExplored ||
        traced.board.toString() != first.board.toString())
        throw std::runtime_error("Trace run disagreed with timed runs");
    const SolverMetric summary = summarizeTrace(traced);
    std::cout << "TRACE\t" << summary.maxDepth << '\t' << summary.guesses << '\t'
              << summary.deadEnds << '\t' << summary.placements << '\t'
              << summary.eliminations << '\t' << summary.events << std::endl;
    return 0;
}

std::wstring executablePath()
{
    std::wstring path(MAX_PATH, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, &path[0], static_cast<DWORD>(path.size()));
    if (length == 0 || length >= path.size()) throw std::runtime_error("Cannot find runner executable path");
    path.resize(length);
    return path;
}

SolverMetric runWorker(const std::wstring& executable, const std::string& scratchFile,
                       const std::string& puzzle, int solver, DWORD timeoutMs,
                       std::size_t maxTimedRuns, double targetTimingMs)
{
    SolverMetric metric;
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    HANDLE output = CreateFileA(scratchFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &attributes,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    if (output == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create worker scratch file");
    HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               &attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (input == INVALID_HANDLE_VALUE)
    {
        CloseHandle(output);
        throw std::runtime_error("Cannot open worker input");
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input;
    startup.hStdOutput = output;
    startup.hStdError = output;
    PROCESS_INFORMATION process{};
    std::wstring command = L'"' + executable + L"\" --worker " + std::to_wstring(solver) +
                           L" " + std::wstring(puzzle.begin(), puzzle.end()) +
                           L" " + std::to_wstring(maxTimedRuns) +
                           L" " + std::to_wstring(static_cast<std::uint64_t>(targetTimingMs));
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');
    const BOOL created = CreateProcessW(executable.c_str(), mutableCommand.data(), nullptr, nullptr,
                                         TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    CloseHandle(input);
    CloseHandle(output);
    if (!created)
    {
        DeleteFileA(scratchFile.c_str());
        metric.status = "launch_error";
        return metric;
    }

    const DWORD wait = WaitForSingleObject(process.hProcess, timeoutMs);
    if (wait == WAIT_TIMEOUT)
    {
        TerminateProcess(process.hProcess, 3);
        WaitForSingleObject(process.hProcess, INFINITE);
        metric.status = "timeout";
    }
    else if (wait == WAIT_OBJECT_0)
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(process.hProcess, &exitCode);
        metric.status = exitCode == 0 ? "ok" : "worker_error";
    }
    else
    {
        TerminateProcess(process.hProcess, 4);
        WaitForSingleObject(process.hProcess, INFINITE);
        metric.status = "wait_error";
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    std::ifstream result(scratchFile);
    std::string line;
    while (std::getline(result, line))
    {
        if (line.compare(0, 7, "TIMING\t") == 0)
        {
            std::istringstream values(line.substr(7));
            metric.timingReady = static_cast<bool>(values >> metric.medianTimeMs >> metric.timedRuns) &&
                                 metric.timedRuns > 0;
        }
        else if (line.compare(0, 6, "TRACE\t") == 0)
        {
            std::istringstream values(line.substr(6));
            metric.traceReady = static_cast<bool>(values >> metric.maxDepth >> metric.guesses >>
                metric.deadEnds >> metric.placements >> metric.eliminations >> metric.events);
        }
    }
    if (metric.status == "ok" && !metric.timingReady) metric.status = "timing_output_error";
    else if (metric.status == "ok" && !metric.traceReady) metric.status = "trace_output_error";
    DeleteFileA(scratchFile.c_str());
    return metric;
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
    escaped += '"';
    return escaped;
}

std::string formatTime(double milliseconds)
{
    std::ostringstream value;
    value << std::fixed << std::setprecision(6) << milliseconds;
    return value.str();
}

int parentMain(const Options& options)
{
    if (!directoryExists(options.dataDirectory))
        throw std::runtime_error("Dataset directory does not exist: " + options.dataDirectory);
    createOutputParentDirectories(options.outputPath);

    std::vector<PuzzleSample> puzzles;
    if (!options.onlyCollection.empty())
    {
        std::vector<PuzzleSample> selected = sampleCollection(joinPath(options.dataDirectory, options.onlyCollection),
                                                               options.sampleSize, options.seed);
        puzzles.insert(puzzles.end(), selected.begin(), selected.end());
    }
    else
    {
        for (const char* name : COLLECTIONS)
        {
            std::vector<PuzzleSample> selected = sampleCollection(joinPath(options.dataDirectory, name),
                                                                   options.sampleSize, options.seed);
            puzzles.insert(puzzles.end(), selected.begin(), selected.end());
        }
    }
    if (puzzles.empty()) throw std::runtime_error("No puzzles selected");

    std::ofstream output(options.outputPath, std::ios::trunc);
    if (!output) throw std::runtime_error("Cannot write CSV: " + options.outputPath);
    output << "Puzzle,Puzzle String,Solver,Time (ms),Samples,Max depth,Guesses,"
              "Dead ends,Placements,Eliminations,Events\n";
    if (!output) throw std::runtime_error("Cannot write CSV header");
    const std::wstring executable = executablePath();
    const std::string scratchFile = options.outputPath + ".worker.tmp";
    std::size_t completed = 0;
    for (const PuzzleSample& puzzle : puzzles)
    {
        for (int solver = 1; solver <= 8; ++solver)
        {
            const SolverMetric metric = runWorker(executable, scratchFile, puzzle.puzzle, solver,
                                                   options.timeoutMs, options.maxTimedRuns,
                                                   options.targetTimingMs);
            std::array<std::string, 11> fields{};
            fields[0] = puzzle.collection + ':' + std::to_string(puzzle.sourceLine);
            fields[1] = puzzle.puzzle;
            fields[2] = SOLVER_NAMES[solver - 1];
            fields[3] = metric.timingReady ? formatTime(metric.medianTimeMs) : metric.status;
            if (metric.timingReady) fields[4] = std::to_string(metric.timedRuns);
            if (metric.traceReady)
            {
                fields[5] = std::to_string(metric.maxDepth);
                fields[6] = std::to_string(metric.guesses);
                fields[7] = std::to_string(metric.deadEnds);
                fields[8] = std::to_string(metric.placements);
                fields[9] = std::to_string(metric.eliminations);
                fields[10] = std::to_string(metric.events);
            }
            else fields[10] = metric.status;
            for (std::size_t index = 0; index < fields.size(); ++index)
            {
                if (index) output << ',';
                output << csv(fields[index]);
            }
            output << '\n';
            output.flush();
            if (!output) throw std::runtime_error("CSV write failed after " + puzzle.collection + ":" +
                                                  std::to_string(puzzle.sourceLine));
        }
        ++completed;
        std::cout << "Processed " << completed << '/' << puzzles.size() << ": "
                  << puzzle.collection << ':' << puzzle.sourceLine << '\n';
    }
    std::cout << "Wrote " << completed * 8 << " solver rows to " << options.outputPath << '\n';
    return 0;
}
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 6 && std::string(argv[1]) == "--worker")
            return workerMain(static_cast<int>(parseUnsigned(argv[2], "solver")), argv[3],
                              static_cast<std::size_t>(parseUnsigned(argv[4], "timed runs")),
                              static_cast<double>(parseUnsigned(argv[5], "timing target")));
        return parentMain(parseOptions(argc, argv));
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
