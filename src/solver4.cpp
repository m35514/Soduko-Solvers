#include "solver4.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedPair.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult Solver4::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair, Heuristics::nakedPair}).solve(board, captureTrace, statistics);
}
