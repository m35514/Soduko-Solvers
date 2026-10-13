#include "xWing.hpp"
#include <array>

namespace
{
struct PairSlots
{
    std::array<int, 2> values{};
    int size = 0;

    void add(int value)
    {
        if (size < 2) values[size] = value;
        ++size;
    }

    bool samePair(const PairSlots& other) const
    {
        return size == 2 && other.size == 2 && values == other.values;
    }
};
}

std::optional<TraceEvent> Heuristics::xWingRows(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    for (int digit = 1; digit <= 9; digit++)
    {
        // For each row, which columns still have this digit as a candidate.
        std::array<PairSlots, 9> candidateColsPerRow{};
        for (int row = 0; row < 9; row++)
        {
            for (int col = 0; col < 9; col++)
            {
                if (board.cells[row][col] != 0) continue;
                if (grid.hasCandidate(row, col, digit))
                {
                    candidateColsPerRow[row].add(col);
                }
            }
        }

        for (int rowA = 0; rowA < 9; rowA++)
        {
            if (candidateColsPerRow[rowA].size != 2) continue;

            for (int rowB = rowA + 1; rowB < 9; rowB++)
            {
                if (!candidateColsPerRow[rowB].samePair(candidateColsPerRow[rowA])) continue;

                // rowA and rowB both confine `digit` to the same two columns.
                int colX = candidateColsPerRow[rowA].values[0];
                int colY = candidateColsPerRow[rowA].values[1];
                TraceEvent event;
                event.technique = TechniqueType::XWingRow;

                for (int row = 0; row < 9; row++)
                {
                    if (row == rowA || row == rowB) continue;
                    for (int col : {colX, colY})
                    {
                        if (board.cells[row][col] != 0) continue;
                        if (grid.hasCandidate(row, col, digit))
                        {
                            grid.eliminate(row, col, digit);
                            event.eliminations.push_back({row, col, digit});
                        }
                    }
                }

                if (!event.eliminations.empty()) return event;
            }
        }
    }
    return std::nullopt;
}

std::optional<TraceEvent> Heuristics::xWingCols(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    for (int digit = 1; digit <= 9; digit++)
    {
        std::array<PairSlots, 9> candidateRowsPerCol{};
        for (int col = 0; col < 9; col++)
        {
            for (int row = 0; row < 9; row++)
            {
                if (board.cells[row][col] != 0) continue;
                if (grid.hasCandidate(row, col, digit))
                {
                    candidateRowsPerCol[col].add(row);
                }
            }
        }

        for (int colA = 0; colA < 9; colA++)
        {
            if (candidateRowsPerCol[colA].size != 2) continue;

            for (int colB = colA + 1; colB < 9; colB++)
            {
                if (!candidateRowsPerCol[colB].samePair(candidateRowsPerCol[colA])) continue;

                int rowX = candidateRowsPerCol[colA].values[0];
                int rowY = candidateRowsPerCol[colA].values[1];
                TraceEvent event;
                event.technique = TechniqueType::XWingCol;

                for (int col = 0; col < 9; col++)
                {
                    if (col == colA || col == colB) continue;
                    for (int row : {rowX, rowY})
                    {
                        if (board.cells[row][col] != 0) continue;
                        if (grid.hasCandidate(row, col, digit))
                        {
                            grid.eliminate(row, col, digit);
                            event.eliminations.push_back({row, col, digit});
                        }
                    }
                }

                if (!event.eliminations.empty()) return event;
            }
        }
    }
    return std::nullopt;
}
