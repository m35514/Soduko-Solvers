#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // A cell with exactly one legal candidate left -- place it.
    std::optional<TraceEvent> nakedSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
