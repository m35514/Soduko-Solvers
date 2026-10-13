#pragma once
#include "new stuff/HeuristicEngine.hpp"

// Array counterpart to Solver 3.  It uses candidates[9][9][10] booleans,
// while Solver 3 continues to use its original bitmask grid.
class ArraySolver3
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
