#pragma once

#include "bitmaskBoard.hpp"
#include "candidateGrid.hpp"
#include "board.hpp"
#include <array>
#include <cstdint>

// Candidate state for the heuristic solvers.  Unlike the original
// CandidateGrid, it owns a BitmaskBoard and uses its row/column/box masks
// for every initial legality check.  During a guess, copying this object also
// snapshots the bitmasks, so undo restores the board facts and mask state
// together.
class BitmaskCandidateGrid : public CandidateGrid
{
public:
    explicit BitmaskCandidateGrid(const Board& board)
        : bitmaskBoard(BitmaskBoard::fromBoard(board))
    {
        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                if (board.cells[row][col] != 0) continue;

                for (int digit = 1; digit <= 9; ++digit)
                {
                    if (bitmaskBoard.isValidPlacement(row, col, digit))
                    {
                        candidates[row][col] |= digitMask(digit);
                    }
                }
            }
        }
    }

    // Converts the internal 9-bit mask to a small fixed candidate list.
    // This deliberately stays array based for every Solver 3-5 call.
    CandidateList candidatesAt(int row, int col) const override
    {
        CandidateList result;
        const std::uint16_t mask = candidates[row][col];
        for (int digit = 1; digit <= 9; ++digit)
        {
            if ((mask & digitMask(digit)) != 0) result.digits[result.size++] = digit;
        }
        return result;
    }

    bool hasCandidate(int row, int col, int digit) const override
    {
        return (candidates[row][col] & digitMask(digit)) != 0;
    }

    int countAt(int row, int col) const override
    {
        std::uint16_t value = candidates[row][col];
        int count = 0;
        while (value != 0)
        {
            value &= static_cast<std::uint16_t>(value - 1);
            ++count;
        }
        return count;
    }

    bool sameCandidates(int firstRow, int firstCol, int secondRow, int secondCol) const override
    {
        return candidates[firstRow][firstCol] == candidates[secondRow][secondCol];
    }

    void eliminate(int row, int col, int digit) override
    {
        candidates[row][col] &= static_cast<std::uint16_t>(~digitMask(digit));
    }

    // The matching Board is updated by placeDigit() immediately before this
    // call.  The BitmaskBoard remains private to the candidate state, so it
    // is updated here at the same time as candidate propagation.
    void collapseAfterPlacement(int row, int col, int digit) override
    {
        bitmaskBoard.place(row, col, digit);
        candidates[row][col] = 0;

        for (int index = 0; index < 9; ++index)
        {
            eliminate(row, index, digit);
            eliminate(index, col, digit);
        }

        const int boxRow = (row / 3) * 3;
        const int boxCol = (col / 3) * 3;
        for (int r = boxRow; r < boxRow + 3; ++r)
        {
            for (int c = boxCol; c < boxCol + 3; ++c)
            {
                eliminate(r, c, digit);
            }
        }
    }

    bool hasContradiction(const Board& board) const override
    {
        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                if (board.cells[row][col] == 0 && candidates[row][col] == 0) return true;
            }
        }
        return false;
    }

private:
    static constexpr std::uint16_t digitMask(int digit)
    {
        return static_cast<std::uint16_t>(1u << (digit - 1));
    }

    BitmaskBoard bitmaskBoard;
    std::array<std::array<std::uint16_t, 9>, 9> candidates{};
};
