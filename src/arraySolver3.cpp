#include "arraySolver3.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult ArraySolver3::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return ArrayHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                                 Heuristics::pointingPair}).solve(board, captureTrace, statistics);
}
