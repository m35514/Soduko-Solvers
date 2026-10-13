#include "solver3.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult Solver3::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair}).solve(board, captureTrace, statistics);
}

SolveResult Solver3::solveWithFirstEmptyCell(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair},
                           CellSelection::findFirstEmptyCell).solve(board, captureTrace, statistics);
}
