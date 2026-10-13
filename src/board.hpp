#pragma once
#include <array>
#include <string>
#include <cstdio>

// Deliberately the simplest possible representation: a plain 9x9 grid of
// ints, 0 = empty. This is the Solver 1 (naive) baseline representation.
//
// Solver 2 uses a SEPARATE struct, BitmaskBoard (see bitmaskBoard.hpp),
// instead of adding bitmask fields here. Board must stay free of bitmask
// information so it remains a clean control group -- otherwise it becomes
// hard to say what each optimization actually contributed.
struct Board
{
    std::array<std::array<int, 9>, 9> cells{};

    // Loads from an 81-character string, row-major, '0' or '.' = empty.
    static Board fromString(const std::string& puzzle)
    {
        Board board;
        for (int i = 0; i < 81; i++)
        {
            char c = puzzle[i];
            int value = (c == '.' || c == '0') ? 0 : (c - '0');
            board.cells[i / 9][i % 9] = value;
        }
        return board;
    }

    std::string toString() const
    {
        std::string result(81, '0');
        for (int row = 0; row < 9; row++)
        {
            for (int col = 0; col < 9; col++)
            {
                result[row * 9 + col] = static_cast<char>('0' + cells[row][col]);
            }
        }
        return result;
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

    // This O(27)-with-early-exit scan is exactly what Solver 2 replaces
    // with bitwise operations. Keep it scan-based on purpose -- it's the
    // thing we are measuring against.
    bool isValidPlacement(int row, int col, int number) const
    {
        // Check the row.
        for (int c = 0; c < 9; c++)
        {
            if (cells[row][c] == number) return false;
        }

        // Check the column.
        for (int r = 0; r < 9; r++)
        {
            if (cells[r][col] == number) return false;
        }

        // Find the top-left corner of the 3x3 box.
        int boxRow = (row / 3) * 3;
        int boxCol = (col / 3) * 3;

        // Check the box.
        for (int r = boxRow; r < boxRow + 3; r++)
        {
            for (int c = boxCol; c < boxCol + 3; c++)
            {
                if (cells[r][c] == number) return false;
            }
        }

        return true;
    }
};
