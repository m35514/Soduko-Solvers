#include "HeuristicEngine.hpp"
#include "placement.hpp"
#include "snapshotProfile.hpp"
#include <chrono>

template <typename Grid>
bool HeuristicEngine<Grid>::isFull(const Board& board)
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (board.cells[row][col] == 0) return false;
        }
    }
    return true;
}

template <typename Grid>
SolveResult HeuristicEngine<Grid>::solve(const Board& startingBoard, bool captureTrace,
                                        SolveStatistics* statistics) const
{
    if (statistics) *statistics = {};
    SolveResult result;
    result.startingBoard = startingBoard;
    result.board = startingBoard;
    auto startTime = std::chrono::high_resolution_clock::now();

    Grid grid(result.board);
    std::vector<TraceEvent>* trace = captureTrace ? &result.trace : nullptr;
    propagate(result.board, grid, result.nodesExplored, trace, 0, 0, statistics);

    if (grid.hasContradiction(result.board))
    {
        result.solved = false;
    }
    else if (isFull(result.board))
    {
        result.solved = true;
    }
    else
    {
        const std::uint64_t stateEventId = trace && !trace->empty() ? trace->back().id : 0;
        result.solved = backtrack(result.board, grid, result.nodesExplored, trace, stateEventId, 0, statistics);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
}

// Tries each technique in order, cheapest/most human-like first, and
// restarts from the top of the list after any change -- exactly the
// "keep re-checking the easy stuff before reaching for the hard stuff"
// order a human solver follows.
template <typename Grid>
void HeuristicEngine<Grid>::propagate(Board& board, Grid& grid, std::uint64_t& nodesExplored,
                                std::vector<TraceEvent>* trace, std::uint64_t branchId, int branchDepth,
                                SolveStatistics* statistics) const
{
    bool progress = true;
    while (progress)
    {
        progress = false;
        for (const Technique& technique : techniques)
        {
            std::optional<TraceEvent> event = technique(board, grid, nodesExplored);
            if (event)
            {
                event->parentId = branchId;
                event->depth = branchDepth;
                appendEvent(trace, std::move(*event), statistics);
                if (grid.hasContradiction(board)) return;
                progress = true;
                break; // restart from the first technique after any change
            }
        }
    }
}

// Reached only once propagation stalls -- no technique in the list made
// any progress. Guesses at the cell chosen by cellSelector (MRV by
// default, or the naive first-empty-cell control strategy), trying only
// candidates the grid already knows are legal. Snapshots BOTH board and
// grid before guessing, since propagate() after the guess can touch many
// cells in both -- undo must restore both wholesale, not cell-by-cell.
template <typename Grid>
bool HeuristicEngine<Grid>::backtrack(Board& board, Grid& grid, std::uint64_t& nodesExplored,
                                std::vector<TraceEvent>* trace, std::uint64_t stateEventId,
                                int activeBranchDepth, SolveStatistics* statistics) const
{
    if (grid.hasContradiction(board)) return false;
    int row, col;
    const std::vector<MrvCellCount> mrvSnapshot = trace ? captureCandidateCounts(board, grid)
                                                        : std::vector<MrvCellCount>{};
    if (!cellSelector(board, grid, row, col))
    {
        return true; // no empty cells left -- solved
    }

    const CandidateGrid::CandidateList candidates = grid.candidatesAt(row, col);

    for (int number : candidates)
    {
#ifdef SUDOKU_PROFILE_SNAPSHOTS
        const auto saveStart = SnapshotProfile::Clock::now();
#endif
        Board boardSnapshot = board;
        Grid gridSnapshot = grid;
#ifdef SUDOKU_PROFILE_SNAPSHOTS
        SnapshotProfile::counters.saveNs += SnapshotProfile::elapsed(saveStart);
        ++SnapshotProfile::counters.saves;
#endif

        // Only a branch/guess contributes to the search-tree node count.
        ++nodesExplored;
        TraceEvent guess;
        guess.parentId = stateEventId;
        guess.depth = activeBranchDepth + 1;
        guess.technique = TechniqueType::Guess;
        guess.placement = {row, col, number};
        guess.mrvSnapshot = mrvSnapshot;
        guess.isGuess = true;
        const std::uint64_t guessId = appendEvent(trace, std::move(guess), statistics);
        const std::size_t guessIndex = trace ? trace->size() - 1 : 0;

        placeDigit(board, grid, row, col, number);
        propagate(board, grid, nodesExplored, trace, guessId, activeBranchDepth + 1, statistics);

        const std::uint64_t nextStateEventId = trace && !trace->empty() ? trace->back().id : 0;
        if (!grid.hasContradiction(board) &&
            (isFull(board) || backtrack(board, grid, nodesExplored, trace, nextStateEventId,
                                        activeBranchDepth + 1, statistics)))
        {
            return true;
        }

#ifdef SUDOKU_PROFILE_SNAPSHOTS
        const auto restoreStart = SnapshotProfile::Clock::now();
#endif
        board = boardSnapshot;
        grid = gridSnapshot;
#ifdef SUDOKU_PROFILE_SNAPSHOTS
        SnapshotProfile::counters.restoreNs += SnapshotProfile::elapsed(restoreStart);
        ++SnapshotProfile::counters.restores;
#endif
        if (statistics) ++statistics->deadEnds;
        if (trace) trace->at(guessIndex).leadsToConflict = true;
        if (trace || statistics)
        {
            TraceEvent backtrack;
            backtrack.parentId = guessId;
            backtrack.depth = activeBranchDepth + 1;
            backtrack.technique = TechniqueType::Backtrack;
            backtrack.placement = {row, col, number};
            if (trace) backtrack.restoredBoard = board.toString();
            appendEvent(trace, std::move(backtrack), statistics);
        }
    }
    return false;
}

template <typename Grid>
std::uint64_t HeuristicEngine<Grid>::appendEvent(std::vector<TraceEvent>* trace, TraceEvent event,
                                               SolveStatistics* statistics)
{
    if (statistics) statistics->recordEvent(event);
    if (trace == nullptr) return 0;
    event.id = trace->size() + 1;
    trace->push_back(std::move(event));
    return trace->back().id;
}

template <typename Grid>
std::vector<MrvCellCount> HeuristicEngine<Grid>::captureCandidateCounts(const Board& board,
                                                                          const CandidateGrid& grid)
{
    std::vector<MrvCellCount> snapshot;
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            if (board.cells[row][col] == 0) snapshot.push_back({row, col, grid.countAt(row, col)});
        }
    }
    return snapshot;
}

template class HeuristicEngine<BitmaskCandidateGrid>;
template class HeuristicEngine<ArrayCandidateGrid>;
