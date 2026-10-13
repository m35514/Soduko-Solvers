#include "nakedPair.hpp"
#include "../candidateSearch.hpp"

std::optional<TraceEvent> Heuristics::nakedPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    const CandidateSearch::UnitType unitTypes[3] = {CandidateSearch::UnitType::Row,
                                                      CandidateSearch::UnitType::Column,
                                                      CandidateSearch::UnitType::Box};

    for (CandidateSearch::UnitType type : unitTypes)
    {
        for (int unitIndex = 0; unitIndex < 9; unitIndex++)
        {
            auto unitCells = CandidateSearch::getUnitCells(unitIndex, type);

            for (std::size_t i = 0; i < unitCells.size(); i++)
            {
                auto [row1, col1] = unitCells[i];
                if (board.cells[row1][col1] != 0) continue;
                if (grid.countAt(row1, col1) != 2) continue;

                for (std::size_t j = i + 1; j < unitCells.size(); j++)
                {
                    auto [row2, col2] = unitCells[j];
                    if (board.cells[row2][col2] != 0) continue;
                    if (!grid.sameCandidates(row2, col2, row1, col1)) continue;

                    // Found a naked pair: (row1,col1) and (row2,col2) share
                    // the same two candidates. Eliminate both digits from
                    // every other cell in this unit.
                    const CandidateGrid::CandidateList pair = grid.candidatesAt(row1, col1);
                    int digitA = pair[0];
                    int digitB = pair[1];
                    TraceEvent event;
                    event.technique = TechniqueType::NakedPair;

                    for (const auto& [row, col] : unitCells)
                    {
                        if ((row == row1 && col == col1) || (row == row2 && col == col2)) continue;
                        if (board.cells[row][col] != 0) continue;

                        if (grid.hasCandidate(row, col, digitA))
                        {
                            grid.eliminate(row, col, digitA);
                            event.eliminations.push_back({row, col, digitA});
                        }
                        if (grid.hasCandidate(row, col, digitB))
                        {
                            grid.eliminate(row, col, digitB);
                            event.eliminations.push_back({row, col, digitB});
                        }
                    }

                    if (!event.eliminations.empty()) return event;
                }
            }
        }
    }
    return std::nullopt;
}
