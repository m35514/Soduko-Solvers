#include "cellSelection.hpp"

bool CellSelection::findFirstEmptyCell(const Board& board, const CandidateGrid& grid, int& row, int& col)
{
    (void)grid; // unused -- this strategy deliberately ignores candidate counts

    for (row = 0; row < 9; row++)
    {
        for (col = 0; col < 9; col++)
        {
            if (board.cells[row][col] == 0) return true;
        }
    }
    return false;
}

bool CellSelection::findMRVCell(const Board& board, const CandidateGrid& grid, int& row, int& col)
{
    int bestRow = -1;
    int bestCol = -1;
    int bestCount = 10; // higher than any possible candidate count (max is 9)

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {
            if (board.cells[r][c] != 0) continue;

            int count = grid.countAt(r, c);
            if (count < bestCount)
            {
                bestCount = count;
                bestRow = r;
                bestCol = c;
            }
        }
    }

    if (bestRow == -1) return false; // no empty cells left

    row = bestRow;
    col = bestCol;
    return true;
}
