#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // Two cells in the same unit share the exact same two candidates --
    // eliminate those two digits from every other cell in that unit.
    // Places nothing; nodesExplored is unused but kept so every technique
    // shares one signature.
    std::optional<TraceEvent> nakedPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
