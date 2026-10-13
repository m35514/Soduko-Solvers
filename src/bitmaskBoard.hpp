#pragma once
#include "board.hpp"
#include <array>
#include <cstdint>
#include <cstdio>

// Board representation for Solver 2: the same 9x9 grid as Board, plus
// three bitmasks (row, column, box) that track which digits are already
// used. Each mask is a uint16_t -- a Sudoku digit is 1-9, so we only need
// bits 1-9, but 16 bits is the smallest convenient integer type that
// comfortably holds them. We are not saying Sudoku has 16 digits; the
// uint16_t is just a container for 9 boolean flags.
//
// This is a SEPARATE struct from Board on purpose (see board.hpp).
struct BitmaskBoard
{
    std::array<std::array<int, 9>, 9> cells{};
    std::array<std::uint16_t, 9> rowMask{};
    std::array<std::uint16_t, 9> colMask{};
    std::array<std::uint16_t, 9> boxMask{};

    // Which 3x3 box does (row, col) belong to? Boxes are numbered 0-8,
    // left to right, top to bottom.
    static int getBoxIndex(int row, int col)
    {
        return (row / 3) * 3 + (col / 3);
    }

    // Builds a BitmaskBoard from a plain Board, filling in the masks to
    // match whatever digits are already placed on the starting grid.
    static BitmaskBoard fromBoard(const Board& source)
    {
        BitmaskBoard board;
        for (int row = 0; row < 9; row++)
        {
            for (int col = 0; col < 9; col++)
            {
                int number = source.cells[row][col];
                board.cells[row][col] = number;

                if (number != 0)
                {
                    std::uint16_t digitBit = 1 << number;
                    board.rowMask[row] |= digitBit;
                    board.colMask[col] |= digitBit;
                    board.boxMask[getBoxIndex(row, col)] |= digitBit;
                }
            }
        }
        return board;
    }

    bool isComplete() const
    {
        for (int row = 0; row < 9; row++)
        {
            for (int col = 0; col < 9; col++)
            {
                if (cells[row][col] == 0) return false;
            }
        }
        return true;
    }

    std::string toString() const
    {
        std::string result(81, '0');
        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                result[row * 9 + col] = static_cast<char>('0' + cells[row][col]);
            }
        }
        return result;
    }

    // The bitmask equivalent of Board::isValidPlacement. Instead of
    // scanning up to 27 cells, this checks three bits.
    bool isValidPlacement(int row, int col, int number) const
    {
        std::uint16_t digitBit = 1 << number;

        bool usedInRow = rowMask[row] & digitBit;
        bool usedInColumn = colMask[col] & digitBit;
        bool usedInBox = boxMask[getBoxIndex(row, col)] & digitBit;

        return !usedInRow && !usedInColumn && !usedInBox;
    }

    // Places a number and updates all three masks. Must be paired with
    // undoPlacement() when backtracking, or the masks go stale.
    void place(int row, int col, int number)
    {
        cells[row][col] = number;

        std::uint16_t digitBit = 1 << number;
        rowMask[row] |= digitBit;
        colMask[col] |= digitBit;
        boxMask[getBoxIndex(row, col)] |= digitBit;
    }

    // Undoes a placement, clearing the bit in all three masks.
    void undoPlacement(int row, int col, int number)
    {
        cells[row][col] = 0;

        std::uint16_t digitBit = 1 << number;
        rowMask[row] &= ~digitBit;
        colMask[col] &= ~digitBit;
        boxMask[getBoxIndex(row, col)] &= ~digitBit;
    }

    void print() const
    {
        for (int row = 0; row < 9; row++)
        {
            for (int col = 0; col < 9; col++)
            {
                char c = (cells[row][col] == 0) ? '.' : static_cast<char>('0' + cells[row][col]);
                putchar(c);
                putchar(' ');
                if (col % 3 == 2 && col != 8) printf("| ");
            }
            putchar('\n');
            if (row % 3 == 2 && row != 8) printf("------+-------+------\n");
        }
    }
};
