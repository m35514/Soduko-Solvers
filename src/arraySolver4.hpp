#pragma once
#include "new stuff/HeuristicEngine.hpp"

class ArraySolver4
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
