#pragma once
#include "bitmaskBoard.hpp"
#include "solveResult.hpp"
#include <chrono>
#include <utility>
#include <vector>

// Solver 2: bitmask backtracking.
//
// Search strategy: IDENTICAL to Solver 1 -- first empty cell, try 1-9 in
// order, backtrack on failure. This stays identical on purpose. The only
// thing this solver changes is HOW legality is checked (bitwise operations
// instead of array scanning). That is what makes Solver 1 vs Solver 2 a
// clean, controlled comparison of representation alone -- if the search
// order were also different, we could no longer say a timing difference
// came from representation rather than from exploring a different tree.
class BitmaskSolver
{
public:
    static SolveResult solve(const Board& startingBoard, bool captureTrace = true,
                             SolveStatistics* statistics = nullptr)
    {
        if (statistics) *statistics = {};
        SolveResult result;
        result.startingBoard = startingBoard;
        BitmaskBoard board = BitmaskBoard::fromBoard(startingBoard);

        auto startTime = std::chrono::high_resolution_clock::now();
        result.solved = backtrack(board, result.nodesExplored,
                                  captureTrace ? &result.trace : nullptr, 0, 0, statistics);
        auto endTime = std::chrono::high_resolution_clock::now();

        result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        result.board.cells = board.cells; // copy final grid into a plain Board for printing
        return result;
    }

private:
    static bool findEmptyCell(const BitmaskBoard& board, int& row, int& col)
    {
        for (row = 0; row < 9; row++)
        {
            for (col = 0; col < 9; col++)
            {
                if (board.cells[row][col] == 0) return true;
            }
        }
        return false;
    }

    static bool backtrack(BitmaskBoard& board, std::uint64_t& nodesExplored,
                          std::vector<TraceEvent>* trace, std::uint64_t parentId, int depth,
                          SolveStatistics* statistics)
    {
        int row, col;
        if (!findEmptyCell(board, row, col))
        {
            return true; // solved
        }

        for (int number = 1; number <= 9; number++)
        {
            const bool legal = board.isValidPlacement(row, col, number);
            TraceEvent event;
            event.parentId = parentId;
            event.depth = depth;
            event.technique = TechniqueType::RawAttempt;
            event.placement = {row, col, number};
            event.isGuess = legal;
            event.valid = legal;
            const std::uint64_t eventId = appendEvent(trace, std::move(event), statistics);
            const std::size_t eventIndex = trace ? trace->size() - 1 : 0;

            if (legal)
            {
                ++nodesExplored; // universal search node: enters a child board state
                board.place(row, col, number);

                if (backtrack(board, nodesExplored, trace, eventId, depth + 1, statistics))
                {
                    return true;
                }

                if (trace) trace->at(eventIndex).leadsToConflict = true;
                if (statistics) ++statistics->deadEnds;
                board.undoPlacement(row, col, number); // undo
                if (trace || statistics)
                {
                    TraceEvent backtrack;
                    backtrack.parentId = eventId;
                    backtrack.depth = depth + 1;
                    backtrack.technique = TechniqueType::Backtrack;
                    backtrack.placement = {row, col, number};
                    if (trace) backtrack.restoredBoard = board.toString();
                    appendEvent(trace, std::move(backtrack), statistics);
                }
            }
        }

        return false;
    }

    static std::uint64_t appendEvent(std::vector<TraceEvent>* trace, TraceEvent event,
                                     SolveStatistics* statistics)
    {
        if (statistics) statistics->recordEvent(event);
        if (trace == nullptr) return 0;
        event.id = trace->size() + 1;
        trace->push_back(std::move(event));
        return trace->back().id;
    }
};
