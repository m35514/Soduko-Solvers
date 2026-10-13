#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"
#include "trace/jsonWriter.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
void require(bool condition, const std::string& message)
{
    if (!condition) throw std::runtime_error(message);
}

SolveResult solve(const Board& board, int solver, bool captureTrace
#ifndef STATISTICS_BASELINE
                  , SolveStatistics* statistics = nullptr
#endif
)
{
#ifdef STATISTICS_BASELINE
#define OPTIONS captureTrace
#else
#define OPTIONS captureTrace, statistics
#endif
    switch (solver)
    {
    case 1: return NaiveSolver::solve(board, OPTIONS);
    case 2: return BitmaskSolver::solve(board, OPTIONS);
    case 3: return Solver3::solve(board, OPTIONS);
    case 4: return Solver4::solve(board, OPTIONS);
    case 5: return Solver5::solve(board, OPTIONS);
    case 6: return ArraySolver3::solve(board, OPTIONS);
    case 7: return ArraySolver4::solve(board, OPTIONS);
    case 8: return ArraySolver5::solve(board, OPTIONS);
    case 9: return Solver3::solveWithFirstEmptyCell(board, OPTIONS);
    default: throw std::runtime_error("Invalid test solver");
    }
#undef OPTIONS
}

std::uint64_t traceDigest(SolveResult& result)
{
    // Ignore elapsed time; compare every serialized board, node and trace field.
    result.timeMs = 0.0;
    const std::string json = TraceJson::toJson(result, "statistics-test");
    std::uint64_t digest = 14695981039346656037ull;
    for (unsigned char character : json)
    {
        digest ^= character;
        digest *= 1099511628211ull;
    }
    return digest;
}

#ifndef STATISTICS_BASELINE
SolveStatistics summarizeTrace(const SolveResult& result)
{
    SolveStatistics expected;
    for (const TraceEvent& event : result.trace)
    {
        expected.maxDepth = std::max(expected.maxDepth, event.depth);
        expected.guesses += event.isGuess;
        expected.deadEnds += event.isGuess && event.leadsToConflict;
        expected.placements += event.placement.has_value();
        expected.eliminations += event.eliminations.size();
    }
    expected.events = result.trace.size();
    return expected;
}

void compareStatistics(const SolveStatistics& expected, const SolveStatistics& actual)
{
    require(expected.maxDepth == actual.maxDepth, "Maximum depth differs");
    require(expected.guesses == actual.guesses, "Guess count differs");
    require(expected.deadEnds == actual.deadEnds, "Dead-end count differs");
    require(expected.placements == actual.placements, "Placement count differs");
    require(expected.eliminations == actual.eliminations, "Elimination count differs");
    require(expected.events == actual.events, "Event count differs");
}

void compareResults(const SolveResult& expected, const SolveResult& actual)
{
    require(expected.solved == actual.solved, "Solved status differs");
    require(expected.board.toString() == actual.board.toString(), "Final board differs");
    require(expected.startingBoard.toString() == actual.startingBoard.toString(), "Starting board differs");
    require(expected.nodesExplored == actual.nodesExplored, "Search node count differs");
}
#endif
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc == 2, "Usage: solve_statistics_test EXAMPLES_FILE");
        std::ifstream input(argv[1]);
        require(static_cast<bool>(input), "Cannot open examples file");
        std::vector<std::string> puzzles;
        std::string line;
        while (std::getline(input, line) && puzzles.size() < 5)
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.size() == 81 && line.find_first_not_of("0123456789.") == std::string::npos)
                puzzles.push_back(line);
        }
        require(puzzles.size() == 5, "Expected five manageable source puzzles");
        const SolveResult solved = solve(Board::fromString(puzzles.front()), 5, false);
        require(solved.solved, "Fixture could not be solved");
        puzzles.push_back(solved.board.toString());
        std::string oneBlank = solved.board.toString();
        oneBlank[0] = '0';
        puzzles.push_back(oneBlank);
        puzzles.push_back("123456780000000009" + std::string(63, '0'));
#ifndef STATISTICS_BASELINE
        bool sawFailedBranch = false;
        bool sawEliminations = false;
        bool sawRawRejection = false;
#endif
        for (std::size_t fixture = 0; fixture < puzzles.size(); ++fixture)
        {
            const Board board = Board::fromString(puzzles[fixture]);
            for (int solver = 1; solver <= 9; ++solver)
            {
                SolveResult traced = solve(board, solver, true);
#ifndef STATISTICS_BASELINE
                const SolveStatistics expected = summarizeTrace(traced);
                // Reuse a nonzero collector to verify each solve resets all its fields.
                SolveStatistics statistics{123, 456, 789, 123, 456, 789};
                const SolveResult counted = solve(board, solver, false, &statistics);
                compareResults(traced, counted);
                compareStatistics(expected, statistics);
                require(counted.trace.empty() && counted.trace.capacity() == 0,
                        "Statistics-only solve retained trace storage");
                const SolveResult plain = solve(board, solver, false);
                compareResults(traced, plain);
                require(plain.trace.empty(), "Plain solve retained a trace");
                SolveResult both = solve(board, solver, true, &statistics);
                compareResults(traced, both);
                compareStatistics(expected, statistics);
                require(traceDigest(traced) == traceDigest(both), "Collector changed the full trace");
                sawFailedBranch = sawFailedBranch || expected.deadEnds > 0;
                sawEliminations = sawEliminations || expected.eliminations > 0;
                for (const TraceEvent& event : traced.trace)
                    sawRawRejection = sawRawRejection || (event.technique == TechniqueType::RawAttempt && !event.valid);
                if (fixture == puzzles.size() - 1) require(!traced.solved, "Expected unsolvable fixture");
#endif
                std::cout << fixture << ',' << solver << ',' << traceDigest(traced) << '\n';
            }
        }
#ifndef STATISTICS_BASELINE
        require(sawFailedBranch && sawEliminations && sawRawRejection, "Required event scenarios were not exercised");
        std::cerr << "PASS: 72 cases; statistics, trace-free solves, collector resets, boards, nodes, full traces.\n";
#endif
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
