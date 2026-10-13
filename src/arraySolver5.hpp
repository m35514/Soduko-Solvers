#pragma once
#include "new stuff/HeuristicEngine.hpp"

class ArraySolver5
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
