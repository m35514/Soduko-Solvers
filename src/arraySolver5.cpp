#include "arraySolver5.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedPair.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"
#include "new stuff/xWing.hpp"

SolveResult ArraySolver5::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return ArrayHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                                 Heuristics::pointingPair, Heuristics::nakedPair,
                                 Heuristics::xWingRows, Heuristics::xWingCols}).solve(board, captureTrace, statistics);
}
