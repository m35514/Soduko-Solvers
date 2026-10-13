#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // A digit that fits only one cell within a row, column, or box --
    // place it there, even if that cell still shows other candidates too.
    std::optional<TraceEvent> hiddenSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
