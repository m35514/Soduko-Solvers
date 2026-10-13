#include "nakedSingle.hpp"
#include "placement.hpp"

std::optional<TraceEvent> Heuristics::nakedSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (board.cells[row][col] != 0) continue;
            if (grid.countAt(row, col) == 1)
            {
                const int number = grid.candidatesAt(row, col)[0];
                (void)nodesExplored;
                TraceEvent event;
                event.technique = TechniqueType::NakedSingle;
                event.placement = placeDigit(board, grid, row, col, number);
                return event;
            }
        }
    }
    return std::nullopt;
}
