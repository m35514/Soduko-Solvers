#pragma once
#include "new stuff/HeuristicEngine.hpp"

class Solver4
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
