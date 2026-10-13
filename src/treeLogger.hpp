#pragma once
#include "board.hpp"
#include "solveResult.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>

// One row of the exported tree file: one attempted digit placement.
//
// The search tree works like this:
//   1. Look at the current board, pick the first empty cell
//   2. Count how many digits 1-9 are legal there (legalOptionCount)
//   3. Try each digit 1-9 in order — one tree node per attempt
//   4. On a legal placement, go deeper; on failure, backtrack up
//
// IMPORTANT: this is NOT used during normal benchmark runs. Writing every
// attempt to memory/disk slows the solver down significantly, which would
// invalidate any timing comparison. Use NaiveSolverWithTreeLog (below)
// only when you specifically want a tree diagram for a small/easy puzzle
// -- for your actual benchmark numbers, keep using the plain NaiveSolver
// and BitmaskSolver from before.
struct TreeNode
{
    std::uint64_t nodeId = 0;
    std::uint64_t parentId = 0; // 0 means "this is a root-level attempt"
    int depth = 0;              // how many cells were already filled at this point
    int row = 0;
    int col = 0;
    int number = 0;
    bool wasLegal = false;
    int legalOptionCount = 0;   // how many of 1-9 were legal at this cell
};

class TreeLogger
{
public:
    std::vector<TreeNode> nodes;

    // Records one attempted placement. Returns the new node's id, so the
    // caller can pass that id down as the parentId for whatever placement
    // comes next in that branch.
    std::uint64_t logAttempt(std::uint64_t parentId, int depth, int row, int col, int number,
                              bool wasLegal, int legalOptionCount)
    {
        std::uint64_t newId = nextId;
        nextId++;

        TreeNode node;
        node.nodeId = newId;
        node.parentId = parentId;
        node.depth = depth;
        node.row = row;
        node.col = col;
        node.number = number;
        node.wasLegal = wasLegal;
        node.legalOptionCount = legalOptionCount;

        nodes.push_back(node);
        return newId;
    }

    // Writes the whole tree out as a plain CSV, one row per attempted node.
    // TreeTraversalVisualizer reads this file and builds the HTML diagram.
    void saveToCsv(const std::string& path) const
    {
        std::ofstream file(path);
        file << "nodeId,parentId,depth,row,col,number,wasLegal,legalOptionCount\n";

        for (const TreeNode& node : nodes)
        {
            file << node.nodeId << ","
                 << node.parentId << ","
                 << node.depth << ","
                 << node.row << ","
                 << node.col << ","
                 << node.number << ","
                 << (node.wasLegal ? 1 : 0) << ","
                 << node.legalOptionCount << "\n";
        }
    }

private:
    std::uint64_t nextId = 1; // 0 is reserved to mean "no parent" (the root)
};

// A copy of Solver 1's backtracking logic, with tree-logging added.
// Deliberately kept as a SEPARATE class from NaiveSolver, rather than
// adding an "if logging enabled" branch inside it -- that would put an
// extra check inside the hottest loop in the program, which is exactly
// the kind of thing that quietly skews a timing benchmark.
class NaiveSolverWithTreeLog
{
public:
    static SolveResult solve(const Board& startingBoard, TreeLogger& logger)
    {
        SolveResult result;
        result.board = startingBoard;

        result.solved = backtrack(result.board, result.nodesExplored, logger, 0, 0);

        return result;
    }

private:
    // The visual trace uses MRV: at every level, one cell is selected and
    // its legal digits become the child branches in the diagram.
    static bool findEmptyCell(const Board& board, int& row, int& col)
    {
        int fewestOptions = 10;
        bool found = false;

        for (int r = 0; r < 9; r++)
        {
            for (int c = 0; c < 9; c++)
            {
                if (board.cells[r][c] != 0)
                {
                    continue;
                }

                int options = 0;
                for (int number = 1; number <= 9; number++)
                {
                    if (board.isValidPlacement(r, c, number)) options++;
                }

                if (options < fewestOptions)
                {
                    fewestOptions = options;
                    row = r;
                    col = c;
                    found = true;
                }
            }
        }

        return found;
    }

    // How many digits 1-9 can legally go in this empty cell right now?
    static int countLegalOptions(const Board& board, int row, int col)
    {
        int count = 0;
        for (int number = 1; number <= 9; number++)
        {
            if (board.isValidPlacement(row, col, number))
            {
                count++;
            }
        }
        return count;
    }

    // parentId: the tree-node id of the placement that led to this call.
    // depth: how many cells are filled so far -- purely for the diagram,
    // not used for any solving decision.
    static bool backtrack(Board& board, std::uint64_t& nodesExplored, TreeLogger& logger,
                           std::uint64_t parentId, int depth)
    {
        int row, col;
        if (!findEmptyCell(board, row, col))
        {
            return true; // solved
        }

        int optionCount = countLegalOptions(board, row, col);

        for (int number = 1; number <= 9; number++)
        {
            nodesExplored++;
            bool legal = board.isValidPlacement(row, col, number);

            std::uint64_t thisNodeId =
                logger.logAttempt(parentId, depth, row, col, number, legal, optionCount);

            if (legal)
            {
                board.cells[row][col] = number;

                if (backtrack(board, nodesExplored, logger, thisNodeId, depth + 1))
                {
                    return true;
                }

                board.cells[row][col] = 0; // undo
            }
        }

        return false;
    }
};
