#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // Within a box, all candidate cells for a digit fall in one row (or
    // column) -- eliminate that digit from the rest of that row/column
    // outside the box. Places nothing; nodesExplored is unused but kept
    // so every technique shares one signature.
    std::optional<TraceEvent> pointingPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
