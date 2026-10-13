#pragma once
#include "board.hpp"
#include "trace/traceEvent.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

// The same totals as summarizing a full trace, without retaining its events.
struct SolveStatistics
{
    int maxDepth = 0;
    std::uint64_t guesses = 0;
    std::uint64_t deadEnds = 0;
    std::uint64_t placements = 0;
    std::uint64_t eliminations = 0;
    std::uint64_t events = 0;

    void recordEvent(const TraceEvent& event)
    {
        maxDepth = std::max(maxDepth, event.depth);
        guesses += event.isGuess;
        placements += event.placement.has_value();
        eliminations += event.eliminations.size();
        ++events;
        // A guess becomes a dead end later, when its branch fails.
    }
};

// Every solver returns this same struct. nodesExplored is universal: it counts
// successful candidate placements that enter a search branch.  Rejected
// legality checks and deterministic deductions are reported separately in the
// trace, so a node means the same thing for every solver implementation.
struct SolveResult
{
    bool solved = false;
    std::uint64_t nodesExplored = 0;
    double timeMs = 0.0;
    // Retained with the final board so a generated trace can replay the solve
    // without reading the dataset again.
    Board startingBoard;
    Board board;
    std::vector<TraceEvent> trace;
};
