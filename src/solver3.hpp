#pragma once
#include "new stuff/HeuristicEngine.hpp"

class Solver3
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
    // Same propagation set as Solver3, but first-empty branching. This is
    // the control run for isolating MRV's effect from the techniques'.
    static SolveResult solveWithFirstEmptyCell(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
