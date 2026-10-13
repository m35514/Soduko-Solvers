#pragma once
#include "board.hpp"
#include "solveResult.hpp"
#include <chrono>
#include <utility>
#include <vector>

// Solver 1: naive backtracking.
//
// Search strategy: always pick the first empty cell (top-left to
// bottom-right), try digits 1-9 in order, backtrack on failure.
// Legality checking: plain array scanning (Board::isValidPlacement).
//
// This is the control group. No bitmasks, no smarter cell choice, no
// heuristics -- on purpose, so later solvers can be compared against it.
class NaiveSolver
{
public:
    static SolveResult solve(const Board& startingBoard, bool captureTrace = true,
                             SolveStatistics* statistics = nullptr)
    {
        if (statistics) *statistics = {};
        SolveResult result;
        result.startingBoard = startingBoard;
        result.board = startingBoard;

        auto startTime = std::chrono::high_resolution_clock::now();
        result.solved = backtrack(result.board, result.nodesExplored,
                                  captureTrace ? &result.trace : nullptr, 0, 0, statistics);
        auto endTime = std::chrono::high_resolution_clock::now();

        result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        return result;
    }

private:
    static bool findEmptyCell(const Board& board, int& row, int& col)
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

    static bool backtrack(Board& board, std::uint64_t& nodesExplored,
                          std::vector<TraceEvent>* trace, std::uint64_t parentId, int depth,
                          SolveStatistics* statistics)
    {
        int row, col;
        if (!findEmptyCell(board, row, col))
        {
            return true; // no empty cells left -- solved
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
                board.cells[row][col] = number;

                if (backtrack(board, nodesExplored, trace, eventId, depth + 1, statistics))
                {
                    return true;
                }

                if (trace) trace->at(eventIndex).leadsToConflict = true;
                if (statistics) ++statistics->deadEnds;
                board.cells[row][col] = 0; // undo -- this number did not work
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

        return false; // triggers backtracking in the caller
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
