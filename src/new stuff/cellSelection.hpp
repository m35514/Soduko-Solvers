#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"

// Strategies for picking which empty cell backtracking should guess on
// next. Both share the same signature so HeuristicEngine can swap between
// them freely: findFirstEmptyCell is the control-group baseline (ignores
// the candidate grid entirely), findMRVCell is the standard choice for
// heuristic solvers.
namespace CellSelection
{
    // First empty cell, scanning top-left to bottom-right.
    bool findFirstEmptyCell(const Board& board, const CandidateGrid& grid, int& row, int& col);

    // The empty cell with the fewest remaining candidates (Minimum
    // Remaining Values). Ties broken by whichever is found first.
    bool findMRVCell(const Board& board, const CandidateGrid& grid, int& row, int& col);
}
