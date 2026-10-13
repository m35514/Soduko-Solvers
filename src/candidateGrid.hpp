#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// Common, non-owning interface used by the deduction techniques.  Both the
// original bitmask grid and the new pure-array grid implement this interface,
// allowing Solvers 3-5 to share their Sudoku techniques while keeping their
// storage representation genuinely separate.
class CandidateGrid
{
public:
    struct CandidateList
    {
        std::array<int, 9> digits{};
        std::size_t size = 0;
        const int* begin() const { return digits.data(); }
        const int* end() const { return digits.data() + size; }
        int operator[](std::size_t index) const { return digits[index]; }
    };

    virtual ~CandidateGrid() = default;
    virtual CandidateList candidatesAt(int row, int col) const = 0;
    virtual bool hasCandidate(int row, int col, int digit) const = 0;
    virtual int countAt(int row, int col) const = 0;
    virtual bool sameCandidates(int firstRow, int firstCol, int secondRow, int secondCol) const = 0;
    virtual void eliminate(int row, int col, int digit) = 0;
    virtual void collapseAfterPlacement(int row, int col, int digit) = 0;
    virtual bool hasContradiction(const struct Board& board) const = 0;
};
