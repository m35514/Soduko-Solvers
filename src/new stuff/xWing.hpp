#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // Row-based X-Wing: a digit is confined to the same two columns in
    // two different rows -- eliminate it from those two columns in every
    // other row. Places nothing; nodesExplored is unused but kept so
    // every technique shares one signature.
    std::optional<TraceEvent> xWingRows(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);

    // Column-based X-Wing: the same idea with rows and columns swapped.
    std::optional<TraceEvent> xWingCols(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
