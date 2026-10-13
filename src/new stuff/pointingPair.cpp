#include "pointingPair.hpp"
#include "../candidateSearch.hpp"

std::optional<TraceEvent> Heuristics::pointingPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    for (int boxIndex = 0; boxIndex < 9; boxIndex++)
    {
        auto boxCells = CandidateSearch::getUnitCells(boxIndex, CandidateSearch::UnitType::Box);

        for (int digit = 1; digit <= 9; digit++)
        {
            int commonRow = -1;
            int commonCol = -1;
            bool sameRow = true;
            bool sameCol = true;
            int matches = 0;

            for (const auto& [row, col] : boxCells)
            {
                if (board.cells[row][col] != 0) continue;
                if (!grid.hasCandidate(row, col, digit)) continue;

                matches++;
                if (commonRow == -1) { commonRow = row; commonCol = col; }
                else
                {
                    if (row != commonRow) sameRow = false;
                    if (col != commonCol) sameCol = false;
                }
            }

            if (matches < 2) continue; // need at least 2 cells to "point"

            TraceEvent event;
            event.technique = TechniqueType::PointingPair;

            if (sameRow)
            {
                for (int col = 0; col < 9; col++)
                {
                    bool inThisBox = (col / 3) == (boxCells[0].second / 3);
                    if (inThisBox) continue;
                    if (board.cells[commonRow][col] != 0) continue;

                    if (grid.hasCandidate(commonRow, col, digit))
                    {
                        grid.eliminate(commonRow, col, digit);
                        event.eliminations.push_back({commonRow, col, digit});
                    }
                }
            }
            else if (sameCol)
            {
                for (int row = 0; row < 9; row++)
                {
                    bool inThisBox = (row / 3) == (boxCells[0].first / 3);
                    if (inThisBox) continue;
                    if (board.cells[row][commonCol] != 0) continue;

                    if (grid.hasCandidate(row, commonCol, digit))
                    {
                        grid.eliminate(row, commonCol, digit);
                        event.eliminations.push_back({row, commonCol, digit});
                    }
                }
            }

            if (!event.eliminations.empty()) return event;
        }
    }
    return std::nullopt;
}
