#pragma once
#include "../board.hpp"
#include "../bitmaskCandidateGrid.hpp"
#include "../arrayCandidateGrid.hpp"
#include "cellSelection.hpp"
#include "../solveResult.hpp"
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

// The one shared solving implementation behind every heuristic solver.
// Owns the BitmaskCandidateGrid, the propagation loop, and backtracking with
// snapshot/undo. Solver3, Solver4, Solver5 etc. are thin wrappers that
// only choose which technique functions get passed in here -- no
// solver-specific solving logic lives outside this file.
template <typename Grid>
class HeuristicEngine
{
public:
    // A technique either places a digit (naked/hidden single) or only
    // eliminates candidates from the grid (naked pair, pointing pair,
    // X-Wing).  A populated event records exactly what it changed.
    using Technique = std::function<std::optional<TraceEvent>(Board&, CandidateGrid&, std::uint64_t&)>;

    // Picks which empty cell backtracking should guess on next.
    using CellSelector = std::function<bool(const Board&, const CandidateGrid&, int&, int&)>;

    explicit HeuristicEngine(std::vector<Technique> techniques,
                              CellSelector cellSelector = CellSelection::findMRVCell)
        : techniques(std::move(techniques)), cellSelector(std::move(cellSelector))
    {
    }

    SolveResult solve(const Board& startingBoard, bool captureTrace = true,
                      SolveStatistics* statistics = nullptr) const;

private:
    std::vector<Technique> techniques;
    CellSelector cellSelector;

    static bool isFull(const Board& board);
    // Deduction does not add search nodes; the counter stays in the
    // signature only because all techniques share one uniform interface.
    void propagate(Board& board, Grid& grid, std::uint64_t& nodesExplored,
                   std::vector<TraceEvent>* trace, std::uint64_t branchId, int branchDepth,
                   SolveStatistics* statistics) const;
    bool backtrack(Board& board, Grid& grid, std::uint64_t& nodesExplored,
                   std::vector<TraceEvent>* trace, std::uint64_t stateEventId,
                   int activeBranchDepth, SolveStatistics* statistics) const;

    static std::uint64_t appendEvent(std::vector<TraceEvent>* trace, TraceEvent event,
                                     SolveStatistics* statistics);
    static std::vector<MrvCellCount> captureCandidateCounts(const Board& board,
                                                            const CandidateGrid& grid);
};

using BitmaskHeuristicEngine = HeuristicEngine<BitmaskCandidateGrid>;
using ArrayHeuristicEngine = HeuristicEngine<ArrayCandidateGrid>;
