#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

// Shared by deduction techniques and backtracking guesses.  A placement is
// not necessarily a search node: deductions are free, while the engine
// counts only a deliberate branch choice.
inline CellDigit placeDigit(Board& board, CandidateGrid& grid, int row, int col, int number)
{
    board.cells[row][col] = number;
    grid.collapseAfterPlacement(row, col, number);
    return {row, col, number};
}
