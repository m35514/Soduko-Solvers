#include "hiddenSingle.hpp"
#include "placement.hpp"
#include "../candidateSearch.hpp"

std::optional<TraceEvent> Heuristics::hiddenSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    const CandidateSearch::UnitType unitTypes[3] = {CandidateSearch::UnitType::Row,
                                                      CandidateSearch::UnitType::Column,
                                                      CandidateSearch::UnitType::Box};

    for (CandidateSearch::UnitType type : unitTypes)
    {
        for (int unitIndex = 0; unitIndex < 9; unitIndex++)
        {
            auto unitCells = CandidateSearch::getUnitCells(unitIndex, type);

            for (int digit = 1; digit <= 9; digit++)
            {
                int count = 0;
                int foundRow = -1;
                int foundCol = -1;

                for (const auto& [row, col] : unitCells)
                {
                    if (grid.hasCandidate(row, col, digit))
                    {
                        count++;
                        foundRow = row;
                        foundCol = col;
                    }
                }

                if (count == 1)
                {
                    (void)nodesExplored;
                    TraceEvent event;
                    event.technique = TechniqueType::HiddenSingle;
                    event.placement = placeDigit(board, grid, foundRow, foundCol, digit);
                    return event;
                }
            }
        }
    }
    return std::nullopt;
}
