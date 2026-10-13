#pragma once

#include "board.hpp"
#include "candidateGrid.hpp"

// A representation for the added array variants of Solvers 3-5.  Each
// candidate is a direct boolean array entry: candidates[row][col][digit].
// No masks or bitwise operators participate in this solver family.
class ArrayCandidateGrid : public CandidateGrid
{
public:
    explicit ArrayCandidateGrid(const Board& board)
    {
        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                if (board.cells[row][col] != 0) continue;
                for (int digit = 1; digit <= 9; ++digit)
                {
                    candidates[row][col][digit] = board.isValidPlacement(row, col, digit);
                }
            }
        }
    }

    CandidateList candidatesAt(int row, int col) const override
    {
        CandidateList result;
        for (int digit = 1; digit <= 9; ++digit)
        {
            if (candidates[row][col][digit]) result.digits[result.size++] = digit;
        }
        return result;
    }

    bool hasCandidate(int row, int col, int digit) const override { return candidates[row][col][digit]; }

    int countAt(int row, int col) const override
    {
        int count = 0;
        for (int digit = 1; digit <= 9; ++digit) count += candidates[row][col][digit];
        return count;
    }

    bool sameCandidates(int firstRow, int firstCol, int secondRow, int secondCol) const override
    {
        for (int digit = 1; digit <= 9; ++digit)
        {
            if (candidates[firstRow][firstCol][digit] != candidates[secondRow][secondCol][digit]) return false;
        }
        return true;
    }

    void eliminate(int row, int col, int digit) override { candidates[row][col][digit] = false; }

    void collapseAfterPlacement(int row, int col, int digit) override
    {
        for (int value = 1; value <= 9; ++value) candidates[row][col][value] = false;
        for (int index = 0; index < 9; ++index)
        {
            candidates[row][index][digit] = false;
            candidates[index][col][digit] = false;
        }
        const int boxRow = row / 3 * 3;
        const int boxCol = col / 3 * 3;
        for (int r = boxRow; r < boxRow + 3; ++r)
            for (int c = boxCol; c < boxCol + 3; ++c) candidates[r][c][digit] = false;
    }

    bool hasContradiction(const Board& board) const override
    {
        for (int row = 0; row < 9; ++row)
            for (int col = 0; col < 9; ++col)
                if (board.cells[row][col] == 0 && countAt(row, col) == 0) return true;
        return false;
    }

private:
    std::array<std::array<std::array<bool, 10>, 9>, 9> candidates{};
};
