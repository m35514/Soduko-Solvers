# Project structure and source code

Full, unchanged contents of the project source, tests, benchmark tools, and build configuration. Datasets, generated results, compiled binaries, IDE state, and temporary files are excluded.

## File structure

```text
sudokku/
|-- src/
|   |-- new stuff/
|   |   |-- cellSelection.cpp
|   |   |-- cellSelection.hpp
|   |   |-- HeuristicEngine.cpp
|   |   |-- HeuristicEngine.hpp
|   |   |-- hiddenSingle.cpp
|   |   |-- hiddenSingle.hpp
|   |   |-- nakedPair.cpp
|   |   |-- nakedPair.hpp
|   |   |-- nakedSingle.cpp
|   |   |-- nakedSingle.hpp
|   |   |-- placement.hpp
|   |   |-- pointingPair.cpp
|   |   |-- pointingPair.hpp
|   |   |-- snapshotProfile.hpp
|   |   |-- xWing.cpp
|   |   `-- xWing.hpp
|   |-- trace/
|   |   |-- jsonWriter.cpp
|   |   |-- jsonWriter.hpp
|   |   `-- traceEvent.hpp
|   |-- arrayCandidateGrid.hpp
|   |-- arraySolver3.cpp
|   |-- arraySolver3.hpp
|   |-- arraySolver4.cpp
|   |-- arraySolver4.hpp
|   |-- arraySolver5.cpp
|   |-- arraySolver5.hpp
|   |-- bitmaskBoard.hpp
|   |-- bitmaskCandidateGrid.hpp
|   |-- board.hpp
|   |-- candidateGrid.hpp
|   |-- candidateSearch.hpp
|   |-- datasetLoader.hpp
|   |-- demo_file_main.cpp
|   |-- demo_main.cpp
|   |-- main.cpp
|   |-- Makefile
|   |-- master_collection_main.cpp
|   |-- solver1.hpp
|   |-- solver2.hpp
|   |-- solver3.cpp
|   |-- solver3.hpp
|   |-- solver4.cpp
|   |-- solver4.hpp
|   |-- solver5.cpp
|   |-- solver5.hpp
|   |-- solveResult.hpp
|   |-- treeLogger.hpp
|   |-- treeTraversalVisualizer.cpp
|   `-- treeTraversalVisualizer.hpp
|-- tests/
|   `-- solve_statistics_test.cpp
|-- tools/
|   |-- analyze_sudoku_benchmarks.py
|   |-- run_snapshot_benchmark.ps1
|   |-- snapshot_cost_benchmark.cpp
|   |-- sudoku_benchmark_sanity.cpp
|   `-- summarize_snapshot_benchmark.ps1
`-- sudoku_solver.cbp
```

## File contents

### src/arrayCandidateGrid.hpp

```cpp
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
```

### src/arraySolver3.cpp

```cpp
#include "arraySolver3.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult ArraySolver3::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return ArrayHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                                 Heuristics::pointingPair}).solve(board, captureTrace, statistics);
}
```

### src/arraySolver3.hpp

```cpp
#pragma once
#include "new stuff/HeuristicEngine.hpp"

// Array counterpart to Solver 3.  It uses candidates[9][9][10] booleans,
// while Solver 3 continues to use its original bitmask grid.
class ArraySolver3
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
```

### src/arraySolver4.cpp

```cpp
#include "arraySolver4.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedPair.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult ArraySolver4::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return ArrayHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                                 Heuristics::pointingPair, Heuristics::nakedPair}).solve(board, captureTrace, statistics);
}
```

### src/arraySolver4.hpp

```cpp
#pragma once
#include "new stuff/HeuristicEngine.hpp"

class ArraySolver4
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
```

### src/arraySolver5.cpp

```cpp
#include "arraySolver5.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedPair.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"
#include "new stuff/xWing.hpp"

SolveResult ArraySolver5::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return ArrayHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                                 Heuristics::pointingPair, Heuristics::nakedPair,
                                 Heuristics::xWingRows, Heuristics::xWingCols}).solve(board, captureTrace, statistics);
}
```

### src/arraySolver5.hpp

```cpp
#pragma once
#include "new stuff/HeuristicEngine.hpp"

class ArraySolver5
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
```

### src/bitmaskBoard.hpp

```cpp
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
```

### src/bitmaskCandidateGrid.hpp

```cpp
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
```

### src/board.hpp

```cpp
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
```

### src/candidateGrid.hpp

```cpp
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
```

### src/candidateSearch.hpp

```cpp
#pragma once

#include <array>
#include <utility>

namespace CandidateSearch
{
    enum class UnitType { Row, Column, Box };

    inline std::array<std::pair<int, int>, 9> getUnitCells(int unitIndex, UnitType type)
    {
        std::array<std::pair<int, int>, 9> cells{};
        for (int index = 0; index < 9; ++index)
        {
            if (type == UnitType::Row) cells[index] = {unitIndex, index};
            else if (type == UnitType::Column) cells[index] = {index, unitIndex};
            else cells[index] = {(unitIndex / 3) * 3 + index / 3,
                                 (unitIndex % 3) * 3 + index % 3};
        }
        return cells;
    }
}
```

### src/datasetLoader.hpp

```cpp
#pragma once
#include "board.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <cstdint>
#include <chrono>

// One row from the dataset: the starting puzzle, the CSV's claimed
// solution (not fully trusted -- see matchesClaimedSolution below),
// and the clue count for that row.
struct PuzzleRecord
{
    Board quiz;
    Board claimedSolution;
    int clueCount = 0;
    std::size_t datasetRow = 0; // one-based CSV row number, including the header
};

class DatasetLoader
{
public:
    // A different seed on every run is the normal mode. Keep the seeded
    // overload below for repeatable benchmarks.
    static std::uint32_t randomSeed()
    {
        std::random_device device;
        const std::uint64_t clock = static_cast<std::uint64_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
        const std::uint32_t entropy = static_cast<std::uint32_t>(device()) ^
                                      (static_cast<std::uint32_t>(device()) << 16U);
        return entropy ^ static_cast<std::uint32_t>(clock) ^
               static_cast<std::uint32_t>(clock >> 32U);
    }

    // Loads the full CSV into memory. For a 670MB / 4M-row file this will
    // take a while and use real RAM -- loadRandomSample() below is usually
    // what you actually want instead.
    static std::vector<PuzzleRecord> loadAll(const std::string& csvPath)
    {
        std::vector<PuzzleRecord> records;
        std::ifstream file(csvPath);

        std::string line;
        std::getline(file, line); // skip header row: quizzes,solutions,clue_numbers

        while (std::getline(file, line))
        {
            PuzzleRecord record;
            if (parseLine(line, record))
            {
                records.push_back(record);
            }
        }

        return records;
    }

    // Loads only puzzles whose clue count falls in [minClues, maxClues].
    // Use this to restrict to the useful 17-35 range instead of the full
    // 17-80 range, where anything above ~40 clues is close to trivial.
    static std::vector<PuzzleRecord> loadFiltered(const std::string& csvPath, int minClues, int maxClues)
    {
        std::vector<PuzzleRecord> records;
        std::ifstream file(csvPath);

        std::string line;
        std::getline(file, line); // skip header

        while (std::getline(file, line))
        {
            PuzzleRecord record;
            if (parseLine(line, record))
            {
                if (record.clueCount >= minClues && record.clueCount <= maxClues)
                {
                    records.push_back(record);
                }
            }
        }

        return records;
    }

    // Loads the whole filtered range, then picks sampleSize random rows
    // from it. Use a fixed seed when you want the same sample across runs
    // (needed if you're comparing solvers on "the same puzzles").
    static std::vector<PuzzleRecord> loadRandomSample(const std::string& csvPath, int minClues, int maxClues,
                                                        std::size_t sampleSize, unsigned int seed)
    {
        std::vector<PuzzleRecord> filtered = loadFiltered(csvPath, minClues, maxClues);

        std::mt19937 randomEngine(seed);
        std::shuffle(filtered.begin(), filtered.end(), randomEngine);

        if (filtered.size() > sampleSize)
        {
            filtered.resize(sampleSize);
        }

        return filtered;
    }

    static std::vector<PuzzleRecord> loadRandomSample(const std::string& csvPath, int minClues, int maxClues,
                                                        std::size_t sampleSize)
    {
        return loadRandomSample(csvPath, minClues, maxClues, sampleSize, randomSeed());
    }

    // Selects one eligible row with reservoir sampling. This is uniform over
    // the filtered rows without loading and shuffling the full dataset.
    static bool loadRandomPuzzle(const std::string& csvPath, int minClues, int maxClues,
                                 unsigned int seed, PuzzleRecord& selected)
    {
        std::ifstream file(csvPath);
        if (!file)
        {
            return false;
        }

        std::string line;
        std::getline(file, line); // header
        std::mt19937 randomEngine(seed);
        std::size_t matchingRows = 0;
        std::size_t datasetRow = 1;

        while (std::getline(file, line))
        {
            datasetRow++;
            PuzzleRecord record;
            if (!parseLine(line, record) || record.clueCount < minClues || record.clueCount > maxClues)
            {
                continue;
            }

            record.datasetRow = datasetRow;
            matchingRows++;
            std::uniform_int_distribution<std::size_t> choose(0, matchingRows - 1);
            if (choose(randomEngine) == 0)
            {
                selected = record;
            }
        }

        return matchingRows > 0;
    }

    // Uniform reservoir sample for batch benchmarks without keeping the CSV
    // or every matching puzzle in memory.
    static std::vector<PuzzleRecord> loadRandomPuzzles(const std::string& csvPath, int minClues, int maxClues,
                                                        std::size_t sampleSize, unsigned int seed)
    {
        std::vector<PuzzleRecord> selected;
        std::ifstream file(csvPath);
        if (!file || sampleSize == 0)
        {
            return selected;
        }

        std::string line;
        std::getline(file, line); // header
        std::mt19937 randomEngine(seed);
        std::size_t matchingRows = 0;
        std::size_t datasetRow = 1;

        while (std::getline(file, line))
        {
            datasetRow++;
            PuzzleRecord record;
            if (!parseLine(line, record) || record.clueCount < minClues || record.clueCount > maxClues)
            {
                continue;
            }

            record.datasetRow = datasetRow;
            matchingRows++;
            if (selected.size() < sampleSize)
            {
                selected.push_back(record);
                continue;
            }

            std::uniform_int_distribution<std::size_t> choose(0, matchingRows - 1);
            const std::size_t replacement = choose(randomEngine);
            if (replacement < sampleSize)
            {
                selected[replacement] = record;
            }
        }

        return selected;
    }

    // Compares a solved board against the CSV's claimed solution. Returns
    // false on any mismatch -- that does NOT automatically mean your
    // solver is wrong. It can mean the CSV row itself is bad (some rows in
    // this dataset are known not to have a unique solution). Treat a
    // mismatch as "investigate this row", not "the solver has a bug".
    static bool matchesClaimedSolution(const Board& solved, const Board& claimedSolution)
    {
        return solved.toString() == claimedSolution.toString();
    }

private:
    static bool parseLine(const std::string& line, PuzzleRecord& outRecord)
    {
        std::stringstream stream(line);
        std::string quizField, solutionField, clueField;

        if (!std::getline(stream, quizField, ',')) return false;
        if (!std::getline(stream, solutionField, ',')) return false;
        if (!std::getline(stream, clueField, ',')) return false;

        if (quizField.size() != 81 || solutionField.size() != 81) return false;

        outRecord.quiz = Board::fromString(quizField);
        outRecord.claimedSolution = Board::fromString(solutionField);
        outRecord.clueCount = std::stoi(clueField);

        return true;
    }
};
```

### src/demo_file_main.cpp

```cpp
#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <windows.h>

namespace
{
const std::filesystem::path PROJECT_ROOT = R"(C:\Users\malob\CLionProjects\sudokku)";
constexpr int BENCHMARK_WARMUP_RUNS = 10;
constexpr int BENCHMARK_TIMED_RUNS = 100;
constexpr double BENCHMARK_TARGET_MS_PER_SOLVER = 1200.0;
volatile std::uint64_t timingGuard = 0;
const std::array<const char*, 8> SOLVER_NAMES = {
    "Naive", "Bitmask", "Solver3 bitmask", "Solver4 bitmask", "Solver5 bitmask",
    "Solver3 array", "Solver4 array", "Solver5 array"
};

struct Options
{
    std::filesystem::path inputPath = PROJECT_ROOT / "src/data/sudokuwiki_examples.txt";
    std::filesystem::path outputPath = PROJECT_ROOT / "results/sudokuwiki_examples_demo.csv";
    bool benchmarkMode = false;
};

struct NamedPuzzle
{
    std::string puzzleName;
    Board board;
};

// Report elapsed time without reading counters while a solver is updating them.
class SolveProgress
{
public:
    explicit SolveProgress(std::string solverLabel) : label(std::move(solverLabel))
    {
        std::cout << this->label << " - Collecting statistics...\n" << std::flush;
        reporter = std::thread([this]
        {
            std::unique_lock<std::mutex> lock(mutex);
            while (!changed.wait_for(lock, std::chrono::seconds(10), [this] { return finished; }))
            {
                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - started).count();
                std::cout << label << " - Still " << stage << " (" << elapsed << " seconds elapsed).\n" << std::flush;
            }
        });
    }

    void benchmarking(std::uint64_t events)
    {
        std::lock_guard<std::mutex> lock(mutex);
        stage = "benchmarking";
        std::cout << label << " - Statistics ready (" << events << " events); benchmarking...\n" << std::flush;
    }

    ~SolveProgress()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            finished = true;
        }
        changed.notify_one();
        if (reporter.joinable()) reporter.join();
    }

    SolveProgress(const SolveProgress&) = delete;
    SolveProgress& operator=(const SolveProgress&) = delete;

private:
    std::string label;
    std::string stage = "collecting statistics";
    const std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
    std::mutex mutex;
    std::condition_variable changed;
    bool finished = false;
    std::thread reporter;
};

void printUsage(const char* executable)
{
    std::cout << "File-based Sudoku demo\n\nUsage: " << executable
              << " [--input FILE] [--output FILE] [--benchmark-mode]\n"
              << "Input: a puzzle name followed by one 81-cell puzzle line; blank lines are optional.\n"
              << "Use 1-9 for clues and 0 or . for blanks; spaces, |, +, and - are allowed.\n"
              << "Runs all eight solvers for every puzzle and writes a CSV, with no trace files.\n"
              << "Default input: " << Options{}.inputPath.string() << '\n'
              << "Default output: " << Options{}.outputPath.string() << '\n';
}

Options parseOptions(int argc, char* argv[])
{
    Options options;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--benchmark-mode") options.benchmarkMode = true;
        else if (argument == "--input" || argument == "--output")
        {
            if (++index >= argc || argv[index][0] == '\0' || std::string(argv[index]).rfind("--", 0) == 0)
                throw std::runtime_error("Missing file path after " + argument);
            if (argument == "--input") options.inputPath = argv[index];
            else options.outputPath = argv[index];
        }
        else throw std::runtime_error("Unknown option: " + argument);
    }
    return options;
}

bool extractPuzzleCells(const std::string& input, std::string& cells, std::string& error)
{
    cells.clear();
    for (unsigned char character : input)
    {
        if (character >= '1' && character <= '9') cells += static_cast<char>(character);
        else if (character == '0' || character == '.') cells += '0';
        else if (std::isspace(character) || character == '|' || character == '+' || character == '-') continue;
        else
        {
            error = std::string("Unsupported character in puzzle input: '") + static_cast<char>(character) + "'.";
            return false;
        }
    }
    return true;
}

bool parsePuzzle(const std::string& input, Board& board, std::string& error)
{
    std::string cells;
    if (!extractPuzzleCells(input, cells, error)) return false;
    if (cells.size() != 81)
    {
        error = "A Sudoku puzzle must contain exactly 81 cells; received " + std::to_string(cells.size()) + ".";
        return false;
    }
    Board checkedBoard;
    for (int index = 0; index < 81; ++index)
    {
        const int digit = cells[index] - '0';
        if (digit == 0) continue;
        const int row = index / 9;
        const int col = index % 9;
        if (!checkedBoard.isValidPlacement(row, col, digit))
        {
            error = "Conflicting given " + std::to_string(digit) + " at row " + std::to_string(row + 1) +
                    ", column " + std::to_string(col + 1) + ".";
            return false;
        }
        checkedBoard.cells[row][col] = digit;
    }
    board = Board::fromString(cells);
    return true;
}

std::string trim(const std::string& line)
{
    const auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char ch) { return std::isspace(ch); });
    const auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    return first < last ? std::string(first, last) : std::string();
}

std::vector<NamedPuzzle> readPuzzles(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open input file: " + path.string());
    std::vector<NamedPuzzle> puzzles;
    std::string puzzleName;
    std::size_t nameLine = 0;
    std::size_t lineNumber = 0;
    std::string line;
    while (std::getline(input, line))
    {
        ++lineNumber;
        if (lineNumber == 1 && line.compare(0, 3, "\xef\xbb\xbf") == 0) line.erase(0, 3);
        line = trim(line);
        if (line.empty()) continue;
        if (puzzleName.empty())
        {
            puzzleName = line;
            nameLine = lineNumber;
            continue;
        }
        Board board;
        std::string error;
        if (!parsePuzzle(line, board, error))
            throw std::runtime_error(path.string() + ": line " + std::to_string(lineNumber) +
                                     ", puzzle '" + puzzleName + "': " + error);
        puzzles.push_back({puzzleName, board});
        puzzleName.clear();
    }
    if (input.bad()) throw std::runtime_error("Cannot read input file: " + path.string());
    if (!puzzleName.empty())
        throw std::runtime_error(path.string() + ": line " + std::to_string(nameLine) +
                                 ", puzzle '" + puzzleName + "': missing puzzle line.");
    if (puzzles.empty()) throw std::runtime_error("Input file contains no puzzles: " + path.string());
    return puzzles;
}

SolveResult solveOnce(const Board& board, int solver, bool captureTrace = false, SolveStatistics* statistics = nullptr)
{
    switch (solver)
    {
    case 1: return NaiveSolver::solve(board, captureTrace, statistics);
    case 2: return BitmaskSolver::solve(board, captureTrace, statistics);
    case 3: return Solver3::solve(board, captureTrace, statistics);
    case 4: return Solver4::solve(board, captureTrace, statistics);
    case 5: return Solver5::solve(board, captureTrace, statistics);
    case 6: return ArraySolver3::solve(board, captureTrace, statistics);
    case 7: return ArraySolver4::solve(board, captureTrace, statistics);
    case 8: return ArraySolver5::solve(board, captureTrace, statistics);
    default: return {};
    }
}

struct BenchmarkResult
{
    double medianMs = 0.0;
    std::size_t timedRuns = 0;
};

BenchmarkResult benchmarkMedian(const Board& board, int solver)
{
    const auto calibrationStart = std::chrono::steady_clock::now();
    timingGuard ^= solveOnce(board, solver, false).nodesExplored;
    const auto calibrationEnd = std::chrono::steady_clock::now();
    const double calibrationMs = std::chrono::duration<double, std::milli>(calibrationEnd - calibrationStart).count();
    const std::size_t timedRuns = std::max<std::size_t>(1, std::min<std::size_t>(
        BENCHMARK_TIMED_RUNS, static_cast<std::size_t>(BENCHMARK_TARGET_MS_PER_SOLVER / std::max(calibrationMs, 0.001))));
    const std::size_t extraWarmups = std::min<std::size_t>(BENCHMARK_WARMUP_RUNS - 1, timedRuns - 1);
    for (std::size_t run = 0; run < extraWarmups; ++run)
        timingGuard ^= solveOnce(board, solver, false).nodesExplored;
    std::vector<double> samples;
    samples.reserve(timedRuns);
    for (std::size_t run = 0; run < timedRuns; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        const SolveResult result = solveOnce(board, solver, false);
        const auto end = std::chrono::steady_clock::now();
        timingGuard ^= result.nodesExplored;
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(samples.begin(), samples.end());
    return {samples[samples.size() / 2], timedRuns};
}

void enableBenchmarkMode()
{
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        std::cerr << "Warning: could not raise process priority for benchmark mode.\n";
    DWORD_PTR processMask = 0;
    DWORD_PTR systemMask = 0;
    if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask) && processMask != 0)
    {
        const DWORD_PTR firstAvailableCore = processMask & (~processMask + 1);
        if (SetThreadAffinityMask(GetCurrentThread(), firstAvailableCore) == 0)
            std::cerr << "Warning: could not pin the benchmark thread to one CPU core.\n";
    }
}

std::string csv(const std::string& value)
{
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string escaped = "\"";
    for (char character : value)
    {
        if (character == '"') escaped += '"';
        escaped += character;
    }
    return escaped + '"';
}

void printComparison(const std::string& puzzleName, const std::array<BenchmarkResult, 8>& benchmarks,
                     const std::array<SolveStatistics, 8>& summaries)
{
    std::cout << '\n' << puzzleName << " - Benchmark comparison (median of trace-free runs after warm-ups; up to "
              << BENCHMARK_TIMED_RUNS << " samples):\n"
              << std::left << std::setw(18) << "Solver" << std::right << std::setw(12) << "Time (ms)"
              << std::setw(9) << "Samples" << std::setw(12) << "Max depth" << std::setw(11) << "Guesses"
              << std::setw(12) << "Dead ends" << std::setw(12) << "Placements"
              << std::setw(14) << "Eliminations" << std::setw(10) << "Events" << '\n';
    for (std::size_t index = 0; index < summaries.size(); ++index)
    {
        const SolveStatistics& summary = summaries[index];
        std::cout << std::left << std::setw(18) << SOLVER_NAMES[index] << std::right << std::fixed << std::setprecision(4)
                  << std::setw(12) << benchmarks[index].medianMs << std::setw(9) << benchmarks[index].timedRuns
                  << std::setw(12) << summary.maxDepth << std::setw(11) << summary.guesses
                  << std::setw(12) << summary.deadEnds << std::setw(12) << summary.placements
                  << std::setw(14) << summary.eliminations << std::setw(10) << summary.events << '\n';
    }
    std::cout << "Slow solvers receive fewer samples to keep one puzzle responsive; Max depth = deepest search branch.\n";
}

int run(const Options& options)
{
    // Validate every record before opening (and potentially overwriting) the CSV.
    const std::vector<NamedPuzzle> puzzles = readPuzzles(options.inputPath);
    if (std::filesystem::exists(options.outputPath) && std::filesystem::equivalent(options.inputPath, options.outputPath))
        throw std::runtime_error("Input and output must be different files.");
    if (options.outputPath.has_parent_path()) std::filesystem::create_directories(options.outputPath.parent_path());
    std::ofstream output(options.outputPath, std::ios::trunc);
    if (!output) throw std::runtime_error("Cannot write CSV: " + options.outputPath.string());
    output << "Puzzle Name,Puzzle String,Solver,Solved,Solution String,Time (ms),Samples,Max depth,"
              "Guesses,Dead ends,Placements,Eliminations,Events\n";
    if (!output) throw std::runtime_error("Cannot write CSV header: " + options.outputPath.string());
    if (options.benchmarkMode)
    {
        enableBenchmarkMode();
        std::cout << "Benchmark mode: high process priority and one-core affinity requested.\n";
    }
    bool allSolved = true;
    std::size_t completed = 0;
    for (const NamedPuzzle& puzzle : puzzles)
    {
        const std::string& puzzleName = puzzle.puzzleName;
        std::cout << '\n' << puzzleName << " - Starting puzzle (" << ++completed << '/' << puzzles.size() << "):\n";
        puzzle.board.print();
        std::cout << std::flush;
        std::array<BenchmarkResult, 8> benchmarks{};
        std::array<SolveStatistics, 8> summaries{};
        Board displayedSolution;
        bool displayedSolved = false;
        for (int solver = 1; solver <= 8; ++solver)
        {
            std::cout << puzzleName << " - Running " << SOLVER_NAMES[solver - 1] << "...\n" << std::flush;
            SolveProgress progress(puzzleName + " / " + SOLVER_NAMES[solver - 1]);
            SolveStatistics summary;
            const SolveResult result = solveOnce(puzzle.board, solver, false, &summary);
            progress.benchmarking(summary.events);
            const BenchmarkResult benchmark = benchmarkMedian(puzzle.board, solver);
            summaries[solver - 1] = summary;
            benchmarks[solver - 1] = benchmark;
            allSolved = allSolved && result.solved;
            if (solver == 5)
            {
                displayedSolution = result.board;
                displayedSolved = result.solved;
            }
            output << csv(puzzleName) << ',' << puzzle.board.toString() << ',' << SOLVER_NAMES[solver - 1] << ','
                   << (result.solved ? "true" : "false") << ',' << (result.solved ? result.board.toString() : "") << ','
                   << std::fixed << std::setprecision(6) << benchmark.medianMs << ',' << benchmark.timedRuns << ','
                   << summary.maxDepth << ',' << summary.guesses << ',' << summary.deadEnds << ','
                   << summary.placements << ',' << summary.eliminations << ',' << summary.events << '\n';
            output.flush();
            if (!output) throw std::runtime_error("CSV write failed for puzzle '" + puzzleName + "'.");
        }
        printComparison(puzzleName, benchmarks, summaries);
        if (displayedSolved)
        {
            std::cout << '\n' << puzzleName << " - First solution found (Solver 5):\n";
            displayedSolution.print();
        }
        else std::cout << '\n' << puzzleName << " - No solution was found by Solver 5.\n";
        std::cout << std::flush;
    }
    output.close();
    if (!output) throw std::runtime_error("Cannot finish CSV: " + options.outputPath.string());
    std::cout << "\nWrote " << puzzles.size() * 8 << " solver rows to " << options.outputPath.string() << '\n';
    return allSolved ? 0 : 2;
}
}

int main(int argc, char* argv[])
{
    try
    {
        for (int index = 1; index < argc; ++index)
        {
            if (std::string(argv[index]) == "--help" || std::string(argv[index]) == "-h")
            {
                printUsage(argv[0]);
                return 0;
            }
        }
        return run(parseOptions(argc, argv));
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

### src/demo_main.cpp

```cpp
#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"
#include "treeTraversalVisualizer.hpp"

#include <cerrno>
#include <array>
#include <chrono>
#include <cctype>
#include <direct.h>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <windows.h>

namespace
{
const std::string RESULTS_DIRECTORY = R"(C:\Users\malob\CLionProjects\sudokku\results)";
constexpr int BENCHMARK_WARMUP_RUNS = 10;
constexpr int BENCHMARK_TIMED_RUNS = 100;
constexpr double BENCHMARK_TARGET_MS_PER_SOLVER = 1200.0;
volatile std::uint64_t timingGuard = 0;

void printUsage(const char* executable)
{
    std::cout
            << "Single-solution Sudoku demo\n\n"
            << "Usage:\n"
            << "  " << executable << "                         Enter a puzzle interactively\n"
            << "  " << executable << " --puzzle <81 cells> --trace-solver <1-8> [--benchmark-mode]\n\n"
            << "Use digits 1-9 for clues and 0 or . for blanks. Spaces, lines, |, +, and - are allowed,\n"
            << "so a normal 9-row Sudoku layout can be pasted directly. Every solver runs once; Solver 5 is the default trace.\n";
}

bool extractPuzzleCells(const std::string& input, std::string& cells, std::string& error)
{
    cells.clear();
    for (unsigned char character : input)
    {
        if (character >= '1' && character <= '9') cells += static_cast<char>(character);
        else if (character == '0' || character == '.') cells += '0';
        else if (std::isspace(character) || character == '|' || character == '+' || character == '-')
        {
            continue;
        }
        else
        {
            error = std::string("Unsupported character in puzzle input: '") +
                    static_cast<char>(character) + "'.";
            return false;
        }
    }
    return true;
}

bool parsePuzzle(const std::string& input, Board& board, std::string& error)
{
    std::string cells;
    if (!extractPuzzleCells(input, cells, error)) return false;
    if (cells.size() != 81)
    {
        error = "A Sudoku puzzle must contain exactly 81 cells; received " + std::to_string(cells.size()) + ".";
        return false;
    }

    Board checkedBoard;
    for (int index = 0; index < 81; ++index)
    {
        const int digit = cells[index] - '0';
        if (digit == 0) continue;

        const int row = index / 9;
        const int col = index % 9;
        if (!checkedBoard.isValidPlacement(row, col, digit))
        {
            error = "Conflicting given " + std::to_string(digit) + " at row " + std::to_string(row + 1) +
                    ", column " + std::to_string(col + 1) + ".";
            return false;
        }
        checkedBoard.cells[row][col] = digit;
    }

    board = Board::fromString(cells);
    return true;
}

bool readPuzzleInteractively(Board& board)
{
    std::cout << "Paste one 81-cell puzzle or nine rows. Use 0 or . for blanks, then press Enter.\n";
    std::string input;
    std::string normalized;
    std::string error;
    while (normalized.size() < 81 && std::cin.good())
    {
        std::string line;
        if (!std::getline(std::cin, line)) break;
        input += line;
        input += '\n';
        if (!extractPuzzleCells(input, normalized, error))
        {
            std::cerr << error << '\n';
            return false;
        }
        if (normalized.size() > 81)
        {
            std::cerr << "A Sudoku puzzle cannot contain more than 81 cells.\n";
            return false;
        }
    }

    if (!parsePuzzle(input, board, error))
    {
        std::cerr << error << '\n';
        return false;
    }
    return true;
}

bool parseSolverNumber(const std::string& value, int& solver)
{
    if (value.size() != 1 || value[0] < '1' || value[0] > '8') return false;
    solver = value[0] - '0';
    return true;
}

const char* solverName(int solver)
{
    switch (solver)
    {
    case 1:
        return "Solver 1 — naive array scan";
    case 2:
        return "Solver 2 — bitmask board";
    case 3:
        return "Solver 3 — bitmask candidates + singles/pointing pairs";
    case 4:
        return "Solver 4 — bitmask candidates + naked pairs";
    case 5:
        return "Solver 5 — bitmask candidates + X-Wing";
    case 6:
        return "Array Solver 3 — array candidates + singles/pointing pairs";
    case 7:
        return "Array Solver 4 — array candidates + naked pairs";
    case 8:
        return "Array Solver 5 — array candidates + X-Wing";
    default:
        return "Unknown solver";
    }
}

SolveResult solveOnce(const Board& board, int solver, bool captureTrace = true)
{
    switch (solver)
    {
    case 1:
        return NaiveSolver::solve(board, captureTrace);
    case 2:
        return BitmaskSolver::solve(board, captureTrace);
    case 3:
        return Solver3::solve(board, captureTrace);
    case 4:
        return Solver4::solve(board, captureTrace);
    case 5:
        return Solver5::solve(board, captureTrace);
    case 6:
        return ArraySolver3::solve(board, captureTrace);
    case 7:
        return ArraySolver4::solve(board, captureTrace);
    case 8:
        return ArraySolver5::solve(board, captureTrace);
    default:
        return {};
    }
}

struct BenchmarkResult
{
    double medianMs = 0.0;
    std::size_t timedRuns = 0;
};

BenchmarkResult benchmarkMedian(const Board& board, int solver)
{
    const auto calibrationStart = std::chrono::steady_clock::now();
    timingGuard ^= solveOnce(board, solver, false).nodesExplored;
    const auto calibrationEnd = std::chrono::steady_clock::now();
    const double calibrationMs = std::chrono::duration<double, std::milli>(calibrationEnd - calibrationStart).count();
    const std::size_t timedRuns = std::max<std::size_t>(1, std::min<std::size_t>(
        BENCHMARK_TIMED_RUNS, static_cast<std::size_t>(BENCHMARK_TARGET_MS_PER_SOLVER / std::max(calibrationMs, 0.001))));
    const std::size_t extraWarmups = std::min<std::size_t>(BENCHMARK_WARMUP_RUNS - 1, timedRuns - 1);
    for (std::size_t run = 0; run < extraWarmups; ++run)
    {
        timingGuard ^= solveOnce(board, solver, false).nodesExplored;
    }

    std::vector<double> samples;
    samples.reserve(timedRuns);
    for (std::size_t run = 0; run < timedRuns; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        const SolveResult result = solveOnce(board, solver, false);
        const auto end = std::chrono::steady_clock::now();
        timingGuard ^= result.nodesExplored;
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(samples.begin(), samples.end());
    return {samples[samples.size() / 2], timedRuns};
}

void enableBenchmarkMode()
{
    // Optional only: keeps routine demonstrations responsive while allowing
    // a user to reduce scheduler migration/noise for a dedicated benchmark.
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
    {
        std::cerr << "Warning: could not raise process priority for benchmark mode.\n";
    }

    DWORD_PTR processMask = 0;
    DWORD_PTR systemMask = 0;
    if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask) && processMask != 0)
    {
        const DWORD_PTR firstAvailableCore = processMask & (~processMask + 1);
        if (SetThreadAffinityMask(GetCurrentThread(), firstAvailableCore) == 0)
        {
            std::cerr << "Warning: could not pin the benchmark thread to one CPU core.\n";
        }
    }
}

bool ensureDirectory(const std::string& path)
{
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

struct ResultSummary
{
    int maxDepth = 0;
    std::size_t guesses = 0;
    std::size_t deadEnds = 0;
    std::size_t placements = 0;
    std::size_t eliminations = 0;
};

ResultSummary summarize(const SolveResult& result)
{
    ResultSummary summary;
    for (const TraceEvent& event : result.trace)
    {
        summary.maxDepth = std::max(summary.maxDepth, event.depth);
        summary.guesses += event.isGuess;
        summary.deadEnds += event.isGuess && event.leadsToConflict;
        summary.placements += event.placement.has_value();
        summary.eliminations += event.eliminations.size();
    }
    return summary;
}

void printComparison(const std::array<SolveResult, 8>& results, const std::array<std::size_t, 8>& timedRuns)
{
    static const std::array<const char*, 8> names =
    {
        "Naive", "Bitmask", "Solver3 bitmask", "Solver4 bitmask", "Solver5 bitmask",
        "Solver3 array", "Solver4 array", "Solver5 array"
    };
    std::cout << "\nBenchmark comparison (median of trace-free runs after warm-ups; up to " << BENCHMARK_TIMED_RUNS << " samples):\n"
              << std::left << std::setw(18) << "Solver" << std::right << std::setw(12) << "Time (ms)"
              << std::setw(9) << "Samples" << std::setw(12) << "Max depth" << std::setw(11) << "Guesses" << std::setw(12) << "Dead ends"
              << std::setw(12) << "Placements" << std::setw(14) << "Eliminations" << std::setw(10) << "Events" << '\n';
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const ResultSummary summary = summarize(results[index]);
        std::cout << std::left << std::setw(18) << names[index] << std::right << std::fixed << std::setprecision(4)
                  << std::setw(12) << results[index].timeMs << std::setw(9) << timedRuns[index] << std::setw(12) << summary.maxDepth
                  << std::setw(11) << summary.guesses << std::setw(12) << summary.deadEnds << std::setw(12) << summary.placements
                  << std::setw(14) << summary.eliminations << std::setw(10) << results[index].trace.size() << '\n';
    }
    std::cout << "Slow solvers receive fewer samples to keep one puzzle responsive; Max depth = deepest search branch.\n";
}
}

int main(int argc, char* argv[])
{
    int traceSolver = 5;
    bool benchmarkMode = false;
    std::string puzzleText;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--help" || argument == "-h")
        {
            printUsage(argv[0]);
            return 0;
        }
        if (argument == "--puzzle")
        {
            if (++index >= argc)
            {
                std::cerr << "--puzzle needs an 81-cell value.\n";
                return 1;
            }
            puzzleText = argv[index];
            continue;
        }
        if (argument == "--trace-solver" || argument == "--solver")
        {
            if (++index >= argc || !parseSolverNumber(argv[index], traceSolver))
            {
                std::cerr << "--trace-solver must be a number from 1 through 8.\n";
                return 1;
            }
            continue;
        }
        if (argument == "--benchmark-mode")
        {
            benchmarkMode = true;
            continue;
        }

        std::cerr << "Unknown option: " << argument << "\n";
        printUsage(argv[0]);
        return 1;
    }

    Board puzzle;
    if (puzzleText.empty())
    {
        if (!readPuzzleInteractively(puzzle)) return 1;
    }
    else
    {
        std::string error;
        if (!parsePuzzle(puzzleText, puzzle, error))
        {
            std::cerr << error << '\n';
            return 1;
        }
    }

    if (benchmarkMode)
    {
        enableBenchmarkMode();
        std::cout << "Benchmark mode: high process priority and one-core affinity requested.\n";
    }

    if (!ensureDirectory(RESULTS_DIRECTORY))
    {
        std::cerr << "Could not create results folder: " << RESULTS_DIRECTORY << '\n';
        return 1;
    }
    const std::string demoDirectory = RESULTS_DIRECTORY + "\\demo";
    if (!ensureDirectory(demoDirectory))
    {
        std::cerr << "Could not create demo output folder: " << demoDirectory << '\n';
        return 1;
    }

    std::cout << "\nCapturing one trace per solver and benchmarking each solver. Starting puzzle:\n";
    puzzle.print();
    std::array<SolveResult, 8> results;
    std::array<std::size_t, 8> timedRuns{};
    for (int solver = 1; solver <= 8; ++solver)
    {
        results[solver - 1] = solveOnce(puzzle, solver);
        const BenchmarkResult benchmark = benchmarkMedian(puzzle, solver);
        results[solver - 1].timeMs = benchmark.medianMs;
        timedRuns[solver - 1] = benchmark.timedRuns;
    }
    printComparison(results, timedRuns);
    const SolveResult& tracedResult = results[traceSolver - 1];

    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch()).count();
    std::array<std::string, 5> traceFiles{};
    for (int solver = 1; solver <= 5; ++solver)
    {
        const std::string baseName = "manual_solver" + std::to_string(solver) + "_" + std::to_string(timestamp);
        const std::string solutionPath = demoDirectory + "\\" + baseName + "_solution_trace.html";
        const std::string runtimePath = demoDirectory + "\\" + baseName + "_full_runtime_trace.html";
        const std::string title = std::string("Manual puzzle · ") + solverName(solver);
        if (!TreeTraversalVisualizer::writeHtml(results[solver - 1], title, solutionPath) ||
                !TreeTraversalVisualizer::writeRuntimeHtml(results[solver - 1], title, runtimePath))
        {
            std::cerr << "Could not write one or more trace files.\n";
            return 1;
        }
        traceFiles[solver - 1] = baseName + "_solution_trace.html";
    }
    const std::string comparisonPath = demoDirectory + "\\manual_solvers_1_to_5_" + std::to_string(timestamp) + "_comparison.html";
    const std::array<SolveResult, 5> standardResults = {results[0], results[1], results[2], results[3], results[4]};
    const std::array<std::string, 5> standardNames = {"solver1", "solver2", "solver3", "solver4", "solver5"};
    if (!TreeTraversalVisualizer::writeComparisonHtml(standardResults, standardNames, traceFiles,
            "Manual puzzle · Solvers 1-5", comparisonPath))
    {
        std::cerr << "Could not write the Solvers 1-5 comparison trace.\n";
        return 1;
    }

    if (tracedResult.solved)
    {
        std::cout << "\nFirst solution found:\n";
        tracedResult.board.print();
    }
    else
    {
        std::cout << "\nNo solution was found for this puzzle. The runtime trace was still written.\n";
    }

    bool allSolved = true;
    for (const SolveResult& result : results) allSolved = allSolved && result.solved;
    std::cout << "\nDetailed trace solver shown above: " << solverName(traceSolver) << "\n"
              << "Solvers 1-5 solution and full-runtime traces: " << demoDirectory << "\n"
              << "Solvers 1-5 comparison: " << comparisonPath << "\n";
    return allSolved ? 0 : 2;
}
```

### src/main.cpp

```cpp
#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "datasetLoader.hpp"
#include "treeTraversalVisualizer.hpp"
#include <cstdio>
#include <array>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

// Adjust the filename at the end of this path if yours differs --
// everything before src/data/ is your confirmed CLion project folder.
const std::string DATASET_PATH = R"(C:\Users\malob\CLionProjects\sudokku\src\data\sudoku_cluewise.csv)";

// Sparse puzzles expose real alternative branches in the MRV visualization.
const int VISUAL_MIN_CLUES = 17;
const int VISUAL_MAX_CLUES = 17;
// Denser puzzles keep a multi-puzzle timing run practical.
const int BATCH_MIN_CLUES = 35;
const int BATCH_MAX_CLUES = 45;

const std::string RESULTS_DIRECTORY = R"(C:\Users\malob\CLionProjects\sudokku\results)";
const std::size_t BENCHMARK_WARMUP_RUNS = 5;
const std::size_t BENCHMARK_TIMED_RUNS = 31;

volatile std::uint64_t timingGuard = 0;

struct ResultSummary
{
    std::size_t guesses = 0;
    std::size_t deadEnds = 0;
    std::size_t placements = 0;
    std::size_t eliminations = 0;
};

static ResultSummary summarizeTrace(const SolveResult& result)
{
    ResultSummary summary;
    for (const TraceEvent& event : result.trace)
    {
        summary.guesses += event.isGuess;
        summary.deadEnds += event.isGuess && event.leadsToConflict;
        summary.placements += event.placement.has_value();
        summary.eliminations += event.eliminations.size();
    }
    return summary;
}

static void printResearchTable(const std::array<SolveResult, 5>& results)
{
    static const std::array<const char*, 5> names = {"Naive", "Bitmask", "Solver3", "Solver4", "Solver5"};
    printf("\nResearch comparison (median of %zu trace-free runs after %zu warm-ups):\n", BENCHMARK_TIMED_RUNS, BENCHMARK_WARMUP_RUNS);
    printf("%-9s %12s %9s %10s %12s %12s %13s %10s\n", "Solver", "Time (ms)", "Nodes", "Guesses", "Dead ends", "Placements", "Eliminations", "Events");
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const ResultSummary summary = summarizeTrace(results[index]);
        printf("%-9s %12.6f %9llu %10zu %12zu %12zu %13zu %10zu\n", names[index], results[index].timeMs,
               static_cast<unsigned long long>(results[index].nodesExplored), summary.guesses, summary.deadEnds,
               summary.placements, summary.eliminations, results[index].trace.size());
    }
    printf("Note: Nodes are successful placements that enter a search branch for every solver.\n");
}

static void writeResearchTable(std::ostream& out, const std::array<SolveResult, 5>& results)
{
    static const std::array<const char*, 5> names = {"Naive", "Bitmask", "Solver3", "Solver4", "Solver5"};
    out << "\nResearch comparison (Nodes are successful placements that enter a search branch)\n"
        << "Solver,Time ms,Nodes,Guesses,Dead ends,Placements,Eliminations,Trace events\n";
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const ResultSummary summary = summarizeTrace(results[index]);
        out << names[index] << ',' << results[index].timeMs << ',' << results[index].nodesExplored << ','
            << summary.guesses << ',' << summary.deadEnds << ',' << summary.placements << ','
            << summary.eliminations << ',' << results[index].trace.size() << '\n';
    }
}

template <typename Solver>
static double benchmarkMedian(const Board& board)
{
    for (std::size_t run = 0; run < BENCHMARK_WARMUP_RUNS; ++run)
    {
        timingGuard ^= Solver::solve(board, false).nodesExplored;
    }
    std::array<double, BENCHMARK_TIMED_RUNS> samples{};
    for (std::size_t run = 0; run < BENCHMARK_TIMED_RUNS; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        timingGuard ^= Solver::solve(board, false).nodesExplored;
        const auto end = std::chrono::steady_clock::now();
        samples[run] = std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

static bool ensureResultsDirectory()
{
    return _mkdir(RESULTS_DIRECTORY.c_str()) == 0 || errno == EEXIST;
}

static bool ensureDirectory(const std::string& path)
{
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

int main(int argc, char* argv[])
{
    const bool isBatch = argc >= 2 && std::string(argv[1]) == "batch";
    const unsigned int seed = isBatch
        ? (argc >= 4 ? static_cast<unsigned int>(std::stoul(argv[3])) : DatasetLoader::randomSeed())
        : (argc >= 2 ? static_cast<unsigned int>(std::stoul(argv[1])) : DatasetLoader::randomSeed());

    if (!ensureResultsDirectory())
    {
        fprintf(stderr, "Could not create results folder: %s\n", RESULTS_DIRECTORY.c_str());
        return 1;
    }

    if (isBatch)
    {
        const std::size_t puzzleCount = argc >= 3 ? static_cast<std::size_t>(std::stoull(argv[2])) : 50;
        const std::vector<PuzzleRecord> puzzles =
            DatasetLoader::loadRandomPuzzles(DATASET_PATH, BATCH_MIN_CLUES, BATCH_MAX_CLUES, puzzleCount, seed);
        if (puzzles.empty())
        {
            fprintf(stderr, "Could not select batch puzzles from %s\n", DATASET_PATH.c_str());
            return 1;
        }

        std::uint64_t naiveNodes = 0;
        std::uint64_t bitmaskNodes = 0;
        std::uint64_t solver3Nodes = 0;
        std::uint64_t solver4Nodes = 0;
        std::uint64_t solver5Nodes = 0;
        double naiveTimeMs = 0.0;
        double bitmaskTimeMs = 0.0;
        double solver3TimeMs = 0.0;
        double solver4TimeMs = 0.0;
        double solver5TimeMs = 0.0;
        int unsolvedResults = 0;
        for (const PuzzleRecord& puzzle : puzzles)
        {
            // Batch mode measures solvers only.  It deliberately omits the
            // trace payload; normal mode captures one inspectable run per solver.
            const SolveResult naive = NaiveSolver::solve(puzzle.quiz, false);
            const SolveResult bitmask = BitmaskSolver::solve(puzzle.quiz, false);
            const SolveResult solver3 = Solver3::solve(puzzle.quiz, false);
            const SolveResult solver4 = Solver4::solve(puzzle.quiz, false);
            const SolveResult solver5 = Solver5::solve(puzzle.quiz, false);
            if (!naive.solved || !bitmask.solved || !solver3.solved || !solver4.solved || !solver5.solved)
            {
                unsolvedResults++;
            }
            naiveNodes += naive.nodesExplored;
            bitmaskNodes += bitmask.nodesExplored;
            solver3Nodes += solver3.nodesExplored;
            solver4Nodes += solver4.nodesExplored;
            solver5Nodes += solver5.nodesExplored;
            naiveTimeMs += benchmarkMedian<NaiveSolver>(puzzle.quiz);
            bitmaskTimeMs += benchmarkMedian<BitmaskSolver>(puzzle.quiz);
            solver3TimeMs += benchmarkMedian<Solver3>(puzzle.quiz);
            solver4TimeMs += benchmarkMedian<Solver4>(puzzle.quiz);
            solver5TimeMs += benchmarkMedian<Solver5>(puzzle.quiz);
        }

        const std::string reportPath = RESULTS_DIRECTORY + "\\batch_seed_" + std::to_string(seed) + ".txt";
        std::ofstream report(reportPath);
        report << std::fixed << std::setprecision(6)
               << "Batch puzzles: " << puzzles.size() << "\nSeed: " << seed << "\n"
               << "Naive: " << naiveNodes << " total nodes, " << naiveTimeMs / puzzles.size() << " ms average\n"
               << "Bitmask: " << bitmaskNodes << " total nodes, " << bitmaskTimeMs / puzzles.size() << " ms average\n"
               << "Solver3: " << solver3Nodes << " total nodes, " << solver3TimeMs / puzzles.size() << " ms average\n"
               << "Solver4: " << solver4Nodes << " total nodes, " << solver4TimeMs / puzzles.size() << " ms average\n"
               << "Solver5: " << solver5Nodes << " total nodes, " << solver5TimeMs / puzzles.size() << " ms average\n"
               << "Unsolved solver results: " << unsolvedResults << "\n";
        printf("Batch of %zu random puzzles\nNaive: %llu nodes, %.6f ms average\nBitmask: %llu nodes, %.6f ms average\nSolver3: %llu nodes, %.6f ms average\nSolver4: %llu nodes, %.6f ms average\nSolver5: %llu nodes, %.6f ms average\nReport: %s\n",
               puzzles.size(), (unsigned long long)naiveNodes, naiveTimeMs / puzzles.size(),
               (unsigned long long)bitmaskNodes, bitmaskTimeMs / puzzles.size(),
               (unsigned long long)solver3Nodes, solver3TimeMs / puzzles.size(),
               (unsigned long long)solver4Nodes, solver4TimeMs / puzzles.size(),
               (unsigned long long)solver5Nodes, solver5TimeMs / puzzles.size(), reportPath.c_str());
        if (unsolvedResults != 0)
        {
            fprintf(stderr, "Warning: at least one solver did not solve %d batch puzzle(s).\n", unsolvedResults);
            return 1;
        }
        return 0;
    }

    PuzzleRecord puzzle;
    printf("Selecting a random puzzle...\n");
    if (!DatasetLoader::loadRandomPuzzle(DATASET_PATH, VISUAL_MIN_CLUES, VISUAL_MAX_CLUES, seed, puzzle))
    {
        fprintf(stderr, "Could not select a puzzle from %s\n", DATASET_PATH.c_str());
        return 1;
    }

    printf("Random puzzle #%zu (seed %u, %d clues):\n", puzzle.datasetRow, seed, puzzle.clueCount);
    puzzle.quiz.print();

    SolveResult naiveResult = NaiveSolver::solve(puzzle.quiz);
    SolveResult bitmaskResult = BitmaskSolver::solve(puzzle.quiz);
    SolveResult solver3Result = Solver3::solve(puzzle.quiz);
    SolveResult solver4Result = Solver4::solve(puzzle.quiz);
    SolveResult solver5Result = Solver5::solve(puzzle.quiz);
    if (!naiveResult.solved || !bitmaskResult.solved || !solver3Result.solved ||
        !solver4Result.solved || !solver5Result.solved)
    {
        fprintf(stderr, "At least one solver could not solve the selected puzzle.\n");
        return 1;
    }
    naiveResult.timeMs = benchmarkMedian<NaiveSolver>(puzzle.quiz);
    bitmaskResult.timeMs = benchmarkMedian<BitmaskSolver>(puzzle.quiz);
    solver3Result.timeMs = benchmarkMedian<Solver3>(puzzle.quiz);
    solver4Result.timeMs = benchmarkMedian<Solver4>(puzzle.quiz);
    solver5Result.timeMs = benchmarkMedian<Solver5>(puzzle.quiz);

    const std::string name = "puzzle_" + std::to_string(puzzle.datasetRow) + "_seed_" + std::to_string(seed);
    const std::string reportPath = RESULTS_DIRECTORY + "\\" + name + ".txt";

    const std::string traceDirectory = RESULTS_DIRECTORY + "\\traces";
    if (!ensureDirectory(traceDirectory))
    {
        fprintf(stderr, "Could not create trace output folder: %s\n", traceDirectory.c_str());
        return 1;
    }
    const std::array<SolveResult, 5> traceResults = {naiveResult, bitmaskResult, solver3Result, solver4Result, solver5Result};
    const std::array<std::string, 5> solverNames = {"solver1", "solver2", "solver3", "solver4", "solver5"};
    const std::array<std::string, 5> traceFilenames = {
        name + "_solver1.html", name + "_solver2.html", name + "_solver3.html", name + "_solver4.html", name + "_solver5.html"};
    const auto writeTrace = [&](const SolveResult& result, const std::string& solverName) {
        return TreeTraversalVisualizer::writeHtml(result, "Puzzle #" + std::to_string(puzzle.datasetRow) + " · " + solverName,
                                                  traceDirectory + "\\" + name + "_" + solverName + ".html");
    };
    if (!writeTrace(naiveResult, "solver1") || !writeTrace(bitmaskResult, "solver2") ||
        !writeTrace(solver3Result, "solver3") || !writeTrace(solver4Result, "solver4") ||
        !writeTrace(solver5Result, "solver5"))
    {
        fprintf(stderr, "Could not create one or more trace visualizations.\n");
        return 1;
    }
    const std::string comparisonPath = traceDirectory + "\\" + name + "_comparison.html";
    if (!TreeTraversalVisualizer::writeComparisonHtml(traceResults, solverNames, traceFilenames,
                                                       "Puzzle #" + std::to_string(puzzle.datasetRow), comparisonPath))
    {
        fprintf(stderr, "Could not create the solver comparison visualization.\n");
        return 1;
    }

    printf("\nCompleted puzzle:\n");
    naiveResult.board.print();
    printf("\nNaive:   %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)naiveResult.nodesExplored, naiveResult.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Bitmask: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)bitmaskResult.nodesExplored, bitmaskResult.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Solver3: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)solver3Result.nodesExplored, solver3Result.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Solver4: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)solver4Result.nodesExplored, solver4Result.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Solver5: %llu nodes, %.6f ms median (%zu timed solves)\n", (unsigned long long)solver5Result.nodesExplored, solver5Result.timeMs, BENCHMARK_TIMED_RUNS);
    printf("Trace visualizations: %s\n", traceDirectory.c_str());
    printf("Comparison visualization: %s\n", comparisonPath.c_str());
    printf("Node counts match: %s\n", naiveResult.nodesExplored == bitmaskResult.nodesExplored ? "yes" : "NO");
    printResearchTable(traceResults);

    std::ofstream report(reportPath);
    report << std::fixed << std::setprecision(6)
           << "Puzzle #" << puzzle.datasetRow << "\n"
           << "Seed: " << seed << "\n"
           << "Clues: " << puzzle.clueCount << "\n"
           << "Puzzle: " << puzzle.quiz.toString() << "\n"
           << "Naive: " << naiveResult.nodesExplored << " nodes, " << naiveResult.timeMs << " ms\n"
           << "Bitmask: " << bitmaskResult.nodesExplored << " nodes, " << bitmaskResult.timeMs << " ms\n"
           << "Solver3: " << solver3Result.nodesExplored << " nodes, " << solver3Result.timeMs << " ms\n"
           << "Solver4: " << solver4Result.nodesExplored << " nodes, " << solver4Result.timeMs << " ms\n"
           << "Solver5: " << solver5Result.nodesExplored << " nodes, " << solver5Result.timeMs << " ms\n"
           << "Trace HTML folder: " << traceDirectory << "\n"
           << "Comparison HTML: " << comparisonPath << "\n";
    writeResearchTable(report, traceResults);

    return 0;
}
```

### src/Makefile

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -march=native
LDFLAGS := -static
SOLVER_BIN := sudoku_solver
DEMO_BIN := sudoku_demo
DEMO_FILE_BIN := sudoku_demo_file
STATISTICS_TEST_BIN := solve_statistics_test

.PHONY: all clean test-statistics

all: $(SOLVER_BIN) $(DEMO_BIN) $(DEMO_FILE_BIN)

SOURCES := main.cpp treeTraversalVisualizer.cpp trace/jsonWriter.cpp solver3.cpp solver4.cpp solver5.cpp arraySolver3.cpp arraySolver4.cpp arraySolver5.cpp \
           new\ stuff/HeuristicEngine.cpp new\ stuff/cellSelection.cpp \
           new\ stuff/nakedSingle.cpp new\ stuff/hiddenSingle.cpp \
           new\ stuff/nakedPair.cpp new\ stuff/pointingPair.cpp new\ stuff/xWing.cpp

$(SOLVER_BIN): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ main.cpp treeTraversalVisualizer.cpp trace/jsonWriter.cpp solver3.cpp solver4.cpp solver5.cpp arraySolver3.cpp arraySolver4.cpp arraySolver5.cpp "new stuff/HeuristicEngine.cpp" "new stuff/cellSelection.cpp" "new stuff/nakedSingle.cpp" "new stuff/hiddenSingle.cpp" "new stuff/nakedPair.cpp" "new stuff/pointingPair.cpp" "new stuff/xWing.cpp"

$(DEMO_BIN): demo_main.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ demo_main.cpp treeTraversalVisualizer.cpp trace/jsonWriter.cpp solver3.cpp solver4.cpp solver5.cpp arraySolver3.cpp arraySolver4.cpp arraySolver5.cpp "new stuff/HeuristicEngine.cpp" "new stuff/cellSelection.cpp" "new stuff/nakedSingle.cpp" "new stuff/hiddenSingle.cpp" "new stuff/nakedPair.cpp" "new stuff/pointingPair.cpp" "new stuff/xWing.cpp"

DEMO_FILE_SOURCES := solver3.cpp solver4.cpp solver5.cpp arraySolver3.cpp arraySolver4.cpp arraySolver5.cpp \
                     new\ stuff/HeuristicEngine.cpp new\ stuff/cellSelection.cpp \
                     new\ stuff/nakedSingle.cpp new\ stuff/hiddenSingle.cpp \
                     new\ stuff/nakedPair.cpp new\ stuff/pointingPair.cpp new\ stuff/xWing.cpp

$(DEMO_FILE_BIN): demo_file_main.cpp $(DEMO_FILE_SOURCES) $(wildcard *.hpp trace/*.hpp) \
                  new\ stuff/HeuristicEngine.hpp new\ stuff/cellSelection.hpp \
                  new\ stuff/nakedSingle.hpp new\ stuff/hiddenSingle.hpp \
                  new\ stuff/nakedPair.hpp new\ stuff/pointingPair.hpp new\ stuff/xWing.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ demo_file_main.cpp solver3.cpp solver4.cpp solver5.cpp arraySolver3.cpp arraySolver4.cpp arraySolver5.cpp "new stuff/HeuristicEngine.cpp" "new stuff/cellSelection.cpp" "new stuff/nakedSingle.cpp" "new stuff/hiddenSingle.cpp" "new stuff/nakedPair.cpp" "new stuff/pointingPair.cpp" "new stuff/xWing.cpp"

$(STATISTICS_TEST_BIN): ../tests/solve_statistics_test.cpp $(DEMO_FILE_SOURCES) trace/jsonWriter.cpp \
                       $(wildcard *.hpp trace/*.hpp) new\ stuff/HeuristicEngine.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -I. -o $@ ../tests/solve_statistics_test.cpp trace/jsonWriter.cpp solver3.cpp solver4.cpp solver5.cpp arraySolver3.cpp arraySolver4.cpp arraySolver5.cpp "new stuff/HeuristicEngine.cpp" "new stuff/cellSelection.cpp" "new stuff/nakedSingle.cpp" "new stuff/hiddenSingle.cpp" "new stuff/nakedPair.cpp" "new stuff/pointingPair.cpp" "new stuff/xWing.cpp"

test-statistics: $(STATISTICS_TEST_BIN)
	./$(STATISTICS_TEST_BIN) data/sudokuwiki_examples.txt

clean:
	rm -f $(SOLVER_BIN) $(DEMO_BIN) $(DEMO_FILE_BIN) $(STATISTICS_TEST_BIN)
```

### src/master_collection_main.cpp

```cpp
#include "board.hpp"
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
// These collections give a repeatable mix of ordinary and technique-rich
// puzzles. The overlapping 17-clue collections are deliberately omitted.
const std::array<const char*, 10> COLLECTIONS = {
    "Sudoku_easy.txt",
    "Sudoku_medium.txt",
    "Sudoku_hard.txt",
    "Sudoku_diabolical.txt",
    "learningcurve_2500.txt",
    "No_One_Trick_Ponies_250.txt",
    "SUPER160.txt",
    "Almost Daily Nightmare_400.txt",
    "Tarek_Pearly6000.txt",
    "Top_50K_Toughest.txt"
};

const std::array<const char*, 8> SOLVER_NAMES = {
    "Naive", "Bitmask", "Solver3_bitmask", "Solver4_bitmask",
    "Solver5_bitmask", "Solver3_array", "Solver4_array", "Solver5_array"
};

volatile std::uint64_t timingGuard = 0;

struct Options
{
    std::string dataDirectory = "src\\data\\SUDOKU Master collection";
    std::string outputPath = "results\\master_collection_benchmark.csv";
    std::size_t sampleSize = 200;
    std::uint32_t seed = 20260928;
    DWORD timeoutMs = 15000;
    std::size_t maxTimedRuns = 31;
    double targetTimingMs = 250.0;
    std::string onlyCollection;
};

struct PuzzleSample
{
    std::string collection;
    std::size_t sourceLine = 0;
    std::string puzzle;
    int clues = 0;
};

struct SolverMetric
{
    std::string status = "error";
    double medianTimeMs = 0.0;
    std::size_t timedRuns = 0;
    int maxDepth = 0;
    std::size_t guesses = 0;
    std::size_t deadEnds = 0;
    std::size_t placements = 0;
    std::size_t eliminations = 0;
    std::size_t events = 0;
    bool timingReady = false;
    bool traceReady = false;
};

void printUsage(const char* executable)
{
    std::cout << "Usage: " << executable << " [--data-dir DIR] [--output FILE]"
              << " [--sample-size N] [--seed N] [--timeout-ms N]"
              << " [--max-timed-runs N] [--target-timing-ms N]"
              << " [--only FILENAME]\n"
              << "Samples each selected collection with a fixed seed and writes one CSV.\n"
              << "Time is the median of trace-free runs; the other measures come\n"
              << "from one in-memory trace per solver. No trace files are saved.\n";
}

std::uint64_t parseUnsigned(const std::string& text, const char* option)
{
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](unsigned char ch) { return std::isdigit(ch); }))
        throw std::runtime_error(std::string("Invalid ") + option + ": " + text);
    std::size_t used = 0;
    const std::uint64_t value = std::stoull(text, &used);
    if (used != text.size()) throw std::runtime_error(std::string("Invalid ") + option + ": " + text);
    return value;
}

Options parseOptions(int argc, char* argv[])
{
    Options options;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--help" || argument == "-h")
        {
            printUsage(argv[0]);
            std::exit(0);
        }
        if (index + 1 >= argc) throw std::runtime_error("Missing value after " + argument);
        const std::string value = argv[++index];
        if (argument == "--data-dir") options.dataDirectory = value;
        else if (argument == "--output") options.outputPath = value;
        else if (argument == "--sample-size") options.sampleSize = static_cast<std::size_t>(parseUnsigned(value, "sample size"));
        else if (argument == "--seed")
        {
            const std::uint64_t parsed = parseUnsigned(value, "seed");
            if (parsed > std::numeric_limits<std::uint32_t>::max()) throw std::runtime_error("Seed is too large");
            options.seed = static_cast<std::uint32_t>(parsed);
        }
        else if (argument == "--timeout-ms")
        {
            const std::uint64_t parsed = parseUnsigned(value, "timeout");
            if (parsed >= INFINITE) throw std::runtime_error("Timeout is too large");
            options.timeoutMs = static_cast<DWORD>(parsed);
        }
        else if (argument == "--max-timed-runs") options.maxTimedRuns = static_cast<std::size_t>(parseUnsigned(value, "timed runs"));
        else if (argument == "--target-timing-ms") options.targetTimingMs = static_cast<double>(parseUnsigned(value, "timing target"));
        else if (argument == "--only") options.onlyCollection = value;
        else throw std::runtime_error("Unknown option: " + argument);
    }
    if (options.sampleSize == 0) throw std::runtime_error("Sample size must be positive");
    if (options.timeoutMs == 0) throw std::runtime_error("Timeout must be positive");
    if (options.maxTimedRuns == 0) throw std::runtime_error("Timed run count must be positive");
    if (options.targetTimingMs <= 0) throw std::runtime_error("Timing target must be positive");
    return options;
}

std::uint32_t filenameHash(const std::string& name)
{
    std::uint32_t hash = 2166136261u;
    for (unsigned char character : name)
    {
        hash ^= character;
        hash *= 16777619u;
    }
    return hash;
}

std::string joinPath(const std::string& directory, const std::string& name)
{
    if (directory.empty()) return name;
    if (directory.back() == '\\' || directory.back() == '/') return directory + name;
    return directory + '\\' + name;
}

std::string filenameOf(const std::string& path)
{
    const std::size_t separator = path.find_last_of("\\/");
    return separator == std::string::npos ? path : path.substr(separator + 1);
}

bool directoryExists(const std::string& path)
{
    const DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void createDirectoryIfNeeded(const std::string& path)
{
    if (path.empty() || directoryExists(path)) return;
    if (!CreateDirectoryA(path.c_str(), nullptr) && !directoryExists(path))
        throw std::runtime_error("Cannot create output directory: " + path);
}

void createOutputParentDirectories(const std::string& outputPath)
{
    const std::size_t lastSeparator = outputPath.find_last_of("\\/");
    if (lastSeparator == std::string::npos || lastSeparator == 0) return;
    std::string parent = outputPath.substr(0, lastSeparator);
    if (lastSeparator == 2 && outputPath[1] == ':') parent = outputPath.substr(0, 3);
    for (std::size_t index = 0; index < parent.size(); ++index)
    {
        if (parent[index] != '\\' && parent[index] != '/') continue;
        if (index == 0 || (index == 2 && parent[1] == ':')) continue;
        createDirectoryIfNeeded(parent.substr(0, index));
    }
    createDirectoryIfNeeded(parent);
}

bool normalizePuzzle(std::string& line, int& clues)
{
    const auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char ch) { return std::isspace(ch); });
    const auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    line = first < last ? std::string(first, last) : std::string();
    if (line.size() != 81) return false;

    clues = 0;
    for (char& character : line)
    {
        if (character == '.') character = '0';
        else if (character < '0' || character > '9') return false;
        if (character != '0') ++clues;
    }

    // Reject contradictory givens before passing a grid to the old solvers.
    std::array<int, 9> rowMask{};
    std::array<int, 9> colMask{};
    std::array<int, 9> boxMask{};
    for (int cell = 0; cell < 81; ++cell)
    {
        const int digit = line[cell] - '0';
        if (digit == 0) continue;
        const int row = cell / 9;
        const int col = cell % 9;
        const int box = (row / 3) * 3 + col / 3;
        const int bit = 1 << digit;
        if ((rowMask[row] | colMask[col] | boxMask[box]) & bit) return false;
        rowMask[row] |= bit;
        colMask[col] |= bit;
        boxMask[box] |= bit;
    }
    return true;
}

bool validCompletedBoard(const Board& result, const Board& startingBoard)
{
    constexpr int allDigits = 0x3fe;
    for (int row = 0; row < 9; ++row)
    {
        int rowMask = 0;
        int colMask = 0;
        for (int col = 0; col < 9; ++col)
        {
            const int rowDigit = result.cells[row][col];
            const int colDigit = result.cells[col][row];
            if (rowDigit < 1 || rowDigit > 9 || colDigit < 1 || colDigit > 9) return false;
            if (startingBoard.cells[row][col] != 0 && startingBoard.cells[row][col] != rowDigit) return false;
            rowMask |= 1 << rowDigit;
            colMask |= 1 << colDigit;
        }
        if (rowMask != allDigits || colMask != allDigits) return false;
    }
    for (int boxRow = 0; boxRow < 9; boxRow += 3)
    {
        for (int boxCol = 0; boxCol < 9; boxCol += 3)
        {
            int mask = 0;
            for (int row = boxRow; row < boxRow + 3; ++row)
                for (int col = boxCol; col < boxCol + 3; ++col)
                    mask |= 1 << result.cells[row][col];
            if (mask != allDigits) return false;
        }
    }
    return true;
}

std::vector<PuzzleSample> sampleCollection(const std::string& path, std::size_t count, std::uint32_t seed)
{
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open dataset: " + path);

    const std::string collection = filenameOf(path);
    std::mt19937 generator(seed ^ filenameHash(collection));
    std::vector<PuzzleSample> selected;
    selected.reserve(count);
    std::size_t eligible = 0;
    std::size_t sourceLine = 0;
    std::size_t skipped = 0;
    std::string line;
    while (std::getline(input, line))
    {
        ++sourceLine;
        int clues = 0;
        if (!normalizePuzzle(line, clues))
        {
            ++skipped;
            continue;
        }
        ++eligible;
        PuzzleSample sample{collection, sourceLine, line, clues};
        if (selected.size() < count) selected.push_back(std::move(sample));
        else
        {
            std::uniform_int_distribution<std::size_t> choose(0, eligible - 1);
            const std::size_t replacement = choose(generator);
            if (replacement < count) selected[replacement] = std::move(sample);
        }
    }
    std::sort(selected.begin(), selected.end(), [](const PuzzleSample& left, const PuzzleSample& right) {
        return left.sourceLine < right.sourceLine;
    });
    std::cout << collection << ": " << eligible << " valid puzzles, " << skipped
              << " skipped lines, " << selected.size() << " selected\n";
    return selected;
}

SolveResult solveOnce(const Board& board, int solver, bool captureTrace)
{
    switch (solver)
    {
    case 1: return NaiveSolver::solve(board, captureTrace);
    case 2: return BitmaskSolver::solve(board, captureTrace);
    case 3: return Solver3::solve(board, captureTrace);
    case 4: return Solver4::solve(board, captureTrace);
    case 5: return Solver5::solve(board, captureTrace);
    case 6: return ArraySolver3::solve(board, captureTrace);
    case 7: return ArraySolver4::solve(board, captureTrace);
    case 8: return ArraySolver5::solve(board, captureTrace);
    default: throw std::runtime_error("Invalid solver number");
    }
}

SolverMetric summarizeTrace(const SolveResult& result)
{
    SolverMetric metric;
    for (const TraceEvent& event : result.trace)
    {
        metric.maxDepth = std::max(metric.maxDepth, event.depth);
        metric.guesses += event.isGuess;
        metric.deadEnds += event.isGuess && event.leadsToConflict;
        metric.placements += event.placement.has_value();
        metric.eliminations += event.eliminations.size();
    }
    metric.events = result.trace.size();
    metric.traceReady = true;
    return metric;
}

int workerMain(int solver, std::string puzzle, std::size_t maxTimedRuns, double targetTimingMs)
{
    int clues = 0;
    if (!normalizePuzzle(puzzle, clues)) return 2;
    const Board board = Board::fromString(puzzle);
    const auto calibrationStart = std::chrono::steady_clock::now();
    const SolveResult first = solveOnce(board, solver, false);
    const auto calibrationEnd = std::chrono::steady_clock::now();
    if (first.solved && !validCompletedBoard(first.board, board))
        throw std::runtime_error("Solver returned an invalid completed board");
    timingGuard ^= first.nodesExplored;
    const double calibrationMs = std::chrono::duration<double, std::milli>(
        calibrationEnd - calibrationStart).count();
    // Preserve one measured solve even if a later repetition or trace times out.
    std::cout << "TIMING\t" << std::fixed << std::setprecision(6)
              << calibrationMs << "\t1" << std::endl;
    const double desiredRuns = targetTimingMs / std::max(calibrationMs, 0.001);
    const std::size_t targetRuns = desiredRuns >= static_cast<double>(maxTimedRuns)
        ? maxTimedRuns : std::max<std::size_t>(1, static_cast<std::size_t>(desiredRuns));
    const std::size_t warmups = std::min<std::size_t>(9, targetRuns - 1);
    for (std::size_t run = 0; run < warmups; ++run)
    {
        const SolveResult warmup = solveOnce(board, solver, false);
        timingGuard ^= warmup.nodesExplored;
    }
    std::vector<double> samples;
    samples.reserve(targetRuns);
    for (std::size_t run = 0; run < targetRuns; ++run)
    {
        const auto start = std::chrono::steady_clock::now();
        const SolveResult result = solveOnce(board, solver, false);
        const auto end = std::chrono::steady_clock::now();
        timingGuard ^= result.nodesExplored;
        if (result.solved != first.solved || result.nodesExplored != first.nodesExplored ||
            result.board.toString() != first.board.toString())
            throw std::runtime_error("Timed runs were inconsistent");
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(samples.begin(), samples.end());
    std::cout << "TIMING\t" << std::fixed << std::setprecision(6)
              << samples[samples.size() / 2] << '\t' << samples.size() << std::endl;

    const SolveResult traced = solveOnce(board, solver, true);
    if (traced.solved != first.solved || traced.nodesExplored != first.nodesExplored ||
        traced.board.toString() != first.board.toString())
        throw std::runtime_error("Trace run disagreed with timed runs");
    const SolverMetric summary = summarizeTrace(traced);
    std::cout << "TRACE\t" << summary.maxDepth << '\t' << summary.guesses << '\t'
              << summary.deadEnds << '\t' << summary.placements << '\t'
              << summary.eliminations << '\t' << summary.events << std::endl;
    return 0;
}

std::wstring executablePath()
{
    std::wstring path(MAX_PATH, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, &path[0], static_cast<DWORD>(path.size()));
    if (length == 0 || length >= path.size()) throw std::runtime_error("Cannot find runner executable path");
    path.resize(length);
    return path;
}

SolverMetric runWorker(const std::wstring& executable, const std::string& scratchFile,
                       const std::string& puzzle, int solver, DWORD timeoutMs,
                       std::size_t maxTimedRuns, double targetTimingMs)
{
    SolverMetric metric;
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    HANDLE output = CreateFileA(scratchFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &attributes,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    if (output == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create worker scratch file");
    HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               &attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (input == INVALID_HANDLE_VALUE)
    {
        CloseHandle(output);
        throw std::runtime_error("Cannot open worker input");
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input;
    startup.hStdOutput = output;
    startup.hStdError = output;
    PROCESS_INFORMATION process{};
    std::wstring command = L'"' + executable + L"\" --worker " + std::to_wstring(solver) +
                           L" " + std::wstring(puzzle.begin(), puzzle.end()) +
                           L" " + std::to_wstring(maxTimedRuns) +
                           L" " + std::to_wstring(static_cast<std::uint64_t>(targetTimingMs));
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');
    const BOOL created = CreateProcessW(executable.c_str(), mutableCommand.data(), nullptr, nullptr,
                                         TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    CloseHandle(input);
    CloseHandle(output);
    if (!created)
    {
        DeleteFileA(scratchFile.c_str());
        metric.status = "launch_error";
        return metric;
    }

    const DWORD wait = WaitForSingleObject(process.hProcess, timeoutMs);
    if (wait == WAIT_TIMEOUT)
    {
        TerminateProcess(process.hProcess, 3);
        WaitForSingleObject(process.hProcess, INFINITE);
        metric.status = "timeout";
    }
    else if (wait == WAIT_OBJECT_0)
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(process.hProcess, &exitCode);
        metric.status = exitCode == 0 ? "ok" : "worker_error";
    }
    else
    {
        TerminateProcess(process.hProcess, 4);
        WaitForSingleObject(process.hProcess, INFINITE);
        metric.status = "wait_error";
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    std::ifstream result(scratchFile);
    std::string line;
    while (std::getline(result, line))
    {
        if (line.compare(0, 7, "TIMING\t") == 0)
        {
            std::istringstream values(line.substr(7));
            metric.timingReady = static_cast<bool>(values >> metric.medianTimeMs >> metric.timedRuns) &&
                                 metric.timedRuns > 0;
        }
        else if (line.compare(0, 6, "TRACE\t") == 0)
        {
            std::istringstream values(line.substr(6));
            metric.traceReady = static_cast<bool>(values >> metric.maxDepth >> metric.guesses >>
                metric.deadEnds >> metric.placements >> metric.eliminations >> metric.events);
        }
    }
    if (metric.status == "ok" && !metric.timingReady) metric.status = "timing_output_error";
    else if (metric.status == "ok" && !metric.traceReady) metric.status = "trace_output_error";
    DeleteFileA(scratchFile.c_str());
    return metric;
}

std::string csv(const std::string& value)
{
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string escaped = "\"";
    for (char character : value)
    {
        if (character == '"') escaped += '"';
        escaped += character;
    }
    escaped += '"';
    return escaped;
}

std::string formatTime(double milliseconds)
{
    std::ostringstream value;
    value << std::fixed << std::setprecision(6) << milliseconds;
    return value.str();
}

int parentMain(const Options& options)
{
    if (!directoryExists(options.dataDirectory))
        throw std::runtime_error("Dataset directory does not exist: " + options.dataDirectory);
    createOutputParentDirectories(options.outputPath);

    std::vector<PuzzleSample> puzzles;
    if (!options.onlyCollection.empty())
    {
        std::vector<PuzzleSample> selected = sampleCollection(joinPath(options.dataDirectory, options.onlyCollection),
                                                               options.sampleSize, options.seed);
        puzzles.insert(puzzles.end(), selected.begin(), selected.end());
    }
    else
    {
        for (const char* name : COLLECTIONS)
        {
            std::vector<PuzzleSample> selected = sampleCollection(joinPath(options.dataDirectory, name),
                                                                   options.sampleSize, options.seed);
            puzzles.insert(puzzles.end(), selected.begin(), selected.end());
        }
    }
    if (puzzles.empty()) throw std::runtime_error("No puzzles selected");

    std::ofstream output(options.outputPath, std::ios::trunc);
    if (!output) throw std::runtime_error("Cannot write CSV: " + options.outputPath);
    output << "Puzzle,Puzzle String,Solver,Time (ms),Samples,Max depth,Guesses,"
              "Dead ends,Placements,Eliminations,Events\n";
    if (!output) throw std::runtime_error("Cannot write CSV header");
    const std::wstring executable = executablePath();
    const std::string scratchFile = options.outputPath + ".worker.tmp";
    std::size_t completed = 0;
    for (const PuzzleSample& puzzle : puzzles)
    {
        for (int solver = 1; solver <= 8; ++solver)
        {
            const SolverMetric metric = runWorker(executable, scratchFile, puzzle.puzzle, solver,
                                                   options.timeoutMs, options.maxTimedRuns,
                                                   options.targetTimingMs);
            std::array<std::string, 11> fields{};
            fields[0] = puzzle.collection + ':' + std::to_string(puzzle.sourceLine);
            fields[1] = puzzle.puzzle;
            fields[2] = SOLVER_NAMES[solver - 1];
            fields[3] = metric.timingReady ? formatTime(metric.medianTimeMs) : metric.status;
            if (metric.timingReady) fields[4] = std::to_string(metric.timedRuns);
            if (metric.traceReady)
            {
                fields[5] = std::to_string(metric.maxDepth);
                fields[6] = std::to_string(metric.guesses);
                fields[7] = std::to_string(metric.deadEnds);
                fields[8] = std::to_string(metric.placements);
                fields[9] = std::to_string(metric.eliminations);
                fields[10] = std::to_string(metric.events);
            }
            else fields[10] = metric.status;
            for (std::size_t index = 0; index < fields.size(); ++index)
            {
                if (index) output << ',';
                output << csv(fields[index]);
            }
            output << '\n';
            output.flush();
            if (!output) throw std::runtime_error("CSV write failed after " + puzzle.collection + ":" +
                                                  std::to_string(puzzle.sourceLine));
        }
        ++completed;
        std::cout << "Processed " << completed << '/' << puzzles.size() << ": "
                  << puzzle.collection << ':' << puzzle.sourceLine << '\n';
    }
    std::cout << "Wrote " << completed * 8 << " solver rows to " << options.outputPath << '\n';
    return 0;
}
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 6 && std::string(argv[1]) == "--worker")
            return workerMain(static_cast<int>(parseUnsigned(argv[2], "solver")), argv[3],
                              static_cast<std::size_t>(parseUnsigned(argv[4], "timed runs")),
                              static_cast<double>(parseUnsigned(argv[5], "timing target")));
        return parentMain(parseOptions(argc, argv));
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

### src/new stuff/cellSelection.cpp

```cpp
#include "cellSelection.hpp"

bool CellSelection::findFirstEmptyCell(const Board& board, const CandidateGrid& grid, int& row, int& col)
{
    (void)grid; // unused -- this strategy deliberately ignores candidate counts

    for (row = 0; row < 9; row++)
    {
        for (col = 0; col < 9; col++)
        {
            if (board.cells[row][col] == 0) return true;
        }
    }
    return false;
}

bool CellSelection::findMRVCell(const Board& board, const CandidateGrid& grid, int& row, int& col)
{
    int bestRow = -1;
    int bestCol = -1;
    int bestCount = 10; // higher than any possible candidate count (max is 9)

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {
            if (board.cells[r][c] != 0) continue;

            int count = grid.countAt(r, c);
            if (count < bestCount)
            {
                bestCount = count;
                bestRow = r;
                bestCol = c;
            }
        }
    }

    if (bestRow == -1) return false; // no empty cells left

    row = bestRow;
    col = bestCol;
    return true;
}
```

### src/new stuff/cellSelection.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"

// Strategies for picking which empty cell backtracking should guess on
// next. Both share the same signature so HeuristicEngine can swap between
// them freely: findFirstEmptyCell is the control-group baseline (ignores
// the candidate grid entirely), findMRVCell is the standard choice for
// heuristic solvers.
namespace CellSelection
{
    // First empty cell, scanning top-left to bottom-right.
    bool findFirstEmptyCell(const Board& board, const CandidateGrid& grid, int& row, int& col);

    // The empty cell with the fewest remaining candidates (Minimum
    // Remaining Values). Ties broken by whichever is found first.
    bool findMRVCell(const Board& board, const CandidateGrid& grid, int& row, int& col);
}
```

### src/new stuff/HeuristicEngine.cpp

```cpp
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
```

### src/new stuff/HeuristicEngine.hpp

```cpp
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
```

### src/new stuff/hiddenSingle.cpp

```cpp
#include "hiddenSingle.hpp"
#include "placement.hpp"
#include "../candidateSearch.hpp"

std::optional<TraceEvent> Heuristics::hiddenSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    const CandidateSearch::UnitType unitTypes[3] = {CandidateSearch::UnitType::Row,
                                                      CandidateSearch::UnitType::Column,
                                                      CandidateSearch::UnitType::Box};

    for (CandidateSearch::UnitType type : unitTypes)
    {
        for (int unitIndex = 0; unitIndex < 9; unitIndex++)
        {
            auto unitCells = CandidateSearch::getUnitCells(unitIndex, type);

            for (int digit = 1; digit <= 9; digit++)
            {
                int count = 0;
                int foundRow = -1;
                int foundCol = -1;

                for (const auto& [row, col] : unitCells)
                {
                    if (grid.hasCandidate(row, col, digit))
                    {
                        count++;
                        foundRow = row;
                        foundCol = col;
                    }
                }

                if (count == 1)
                {
                    (void)nodesExplored;
                    TraceEvent event;
                    event.technique = TechniqueType::HiddenSingle;
                    event.placement = placeDigit(board, grid, foundRow, foundCol, digit);
                    return event;
                }
            }
        }
    }
    return std::nullopt;
}
```

### src/new stuff/hiddenSingle.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // A digit that fits only one cell within a row, column, or box --
    // place it there, even if that cell still shows other candidates too.
    std::optional<TraceEvent> hiddenSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
```

### src/new stuff/nakedPair.cpp

```cpp
#include "nakedPair.hpp"
#include "../candidateSearch.hpp"

std::optional<TraceEvent> Heuristics::nakedPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    const CandidateSearch::UnitType unitTypes[3] = {CandidateSearch::UnitType::Row,
                                                      CandidateSearch::UnitType::Column,
                                                      CandidateSearch::UnitType::Box};

    for (CandidateSearch::UnitType type : unitTypes)
    {
        for (int unitIndex = 0; unitIndex < 9; unitIndex++)
        {
            auto unitCells = CandidateSearch::getUnitCells(unitIndex, type);

            for (std::size_t i = 0; i < unitCells.size(); i++)
            {
                auto [row1, col1] = unitCells[i];
                if (board.cells[row1][col1] != 0) continue;
                if (grid.countAt(row1, col1) != 2) continue;

                for (std::size_t j = i + 1; j < unitCells.size(); j++)
                {
                    auto [row2, col2] = unitCells[j];
                    if (board.cells[row2][col2] != 0) continue;
                    if (!grid.sameCandidates(row2, col2, row1, col1)) continue;

                    // Found a naked pair: (row1,col1) and (row2,col2) share
                    // the same two candidates. Eliminate both digits from
                    // every other cell in this unit.
                    const CandidateGrid::CandidateList pair = grid.candidatesAt(row1, col1);
                    int digitA = pair[0];
                    int digitB = pair[1];
                    TraceEvent event;
                    event.technique = TechniqueType::NakedPair;

                    for (const auto& [row, col] : unitCells)
                    {
                        if ((row == row1 && col == col1) || (row == row2 && col == col2)) continue;
                        if (board.cells[row][col] != 0) continue;

                        if (grid.hasCandidate(row, col, digitA))
                        {
                            grid.eliminate(row, col, digitA);
                            event.eliminations.push_back({row, col, digitA});
                        }
                        if (grid.hasCandidate(row, col, digitB))
                        {
                            grid.eliminate(row, col, digitB);
                            event.eliminations.push_back({row, col, digitB});
                        }
                    }

                    if (!event.eliminations.empty()) return event;
                }
            }
        }
    }
    return std::nullopt;
}
```

### src/new stuff/nakedPair.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // Two cells in the same unit share the exact same two candidates --
    // eliminate those two digits from every other cell in that unit.
    // Places nothing; nodesExplored is unused but kept so every technique
    // shares one signature.
    std::optional<TraceEvent> nakedPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
```

### src/new stuff/nakedSingle.cpp

```cpp
#include "nakedSingle.hpp"
#include "placement.hpp"

std::optional<TraceEvent> Heuristics::nakedSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (board.cells[row][col] != 0) continue;
            if (grid.countAt(row, col) == 1)
            {
                const int number = grid.candidatesAt(row, col)[0];
                (void)nodesExplored;
                TraceEvent event;
                event.technique = TechniqueType::NakedSingle;
                event.placement = placeDigit(board, grid, row, col, number);
                return event;
            }
        }
    }
    return std::nullopt;
}
```

### src/new stuff/nakedSingle.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // A cell with exactly one legal candidate left -- place it.
    std::optional<TraceEvent> nakedSingle(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
```

### src/new stuff/placement.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

// Shared by deduction techniques and backtracking guesses.  A placement is
// not necessarily a search node: deductions are free, while the engine
// counts only a deliberate branch choice.
inline CellDigit placeDigit(Board& board, CandidateGrid& grid, int row, int col, int number)
{
    board.cells[row][col] = number;
    grid.collapseAfterPlacement(row, col, number);
    return {row, col, number};
}
```

### src/new stuff/pointingPair.cpp

```cpp
#include "pointingPair.hpp"
#include "../candidateSearch.hpp"

std::optional<TraceEvent> Heuristics::pointingPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    for (int boxIndex = 0; boxIndex < 9; boxIndex++)
    {
        auto boxCells = CandidateSearch::getUnitCells(boxIndex, CandidateSearch::UnitType::Box);

        for (int digit = 1; digit <= 9; digit++)
        {
            int commonRow = -1;
            int commonCol = -1;
            bool sameRow = true;
            bool sameCol = true;
            int matches = 0;

            for (const auto& [row, col] : boxCells)
            {
                if (board.cells[row][col] != 0) continue;
                if (!grid.hasCandidate(row, col, digit)) continue;

                matches++;
                if (commonRow == -1) { commonRow = row; commonCol = col; }
                else
                {
                    if (row != commonRow) sameRow = false;
                    if (col != commonCol) sameCol = false;
                }
            }

            if (matches < 2) continue; // need at least 2 cells to "point"

            TraceEvent event;
            event.technique = TechniqueType::PointingPair;

            if (sameRow)
            {
                for (int col = 0; col < 9; col++)
                {
                    bool inThisBox = (col / 3) == (boxCells[0].second / 3);
                    if (inThisBox) continue;
                    if (board.cells[commonRow][col] != 0) continue;

                    if (grid.hasCandidate(commonRow, col, digit))
                    {
                        grid.eliminate(commonRow, col, digit);
                        event.eliminations.push_back({commonRow, col, digit});
                    }
                }
            }
            else if (sameCol)
            {
                for (int row = 0; row < 9; row++)
                {
                    bool inThisBox = (row / 3) == (boxCells[0].first / 3);
                    if (inThisBox) continue;
                    if (board.cells[row][commonCol] != 0) continue;

                    if (grid.hasCandidate(row, commonCol, digit))
                    {
                        grid.eliminate(row, commonCol, digit);
                        event.eliminations.push_back({row, commonCol, digit});
                    }
                }
            }

            if (!event.eliminations.empty()) return event;
        }
    }
    return std::nullopt;
}
```

### src/new stuff/pointingPair.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // Within a box, all candidate cells for a digit fall in one row (or
    // column) -- eliminate that digit from the rest of that row/column
    // outside the box. Places nothing; nodesExplored is unused but kept
    // so every technique shares one signature.
    std::optional<TraceEvent> pointingPair(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
```

### src/new stuff/snapshotProfile.hpp

```cpp
#pragma once

// Opt-in benchmark instrumentation. Normal builds contain no timing calls.
#ifdef SUDOKU_PROFILE_SNAPSHOTS
#include <chrono>
#include <cstdint>
namespace SnapshotProfile
{
using Clock = std::chrono::steady_clock;
struct Counters
{
    std::uint64_t saves = 0, restores = 0;
    double saveNs = 0, restoreNs = 0;
};
inline thread_local Counters counters;
inline double elapsed(Clock::time_point start)
{
    return std::chrono::duration<double, std::nano>(Clock::now() - start).count();
}
}
#endif
```

### src/new stuff/xWing.cpp

```cpp
#include "xWing.hpp"
#include <array>

namespace
{
struct PairSlots
{
    std::array<int, 2> values{};
    int size = 0;

    void add(int value)
    {
        if (size < 2) values[size] = value;
        ++size;
    }

    bool samePair(const PairSlots& other) const
    {
        return size == 2 && other.size == 2 && values == other.values;
    }
};
}

std::optional<TraceEvent> Heuristics::xWingRows(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    for (int digit = 1; digit <= 9; digit++)
    {
        // For each row, which columns still have this digit as a candidate.
        std::array<PairSlots, 9> candidateColsPerRow{};
        for (int row = 0; row < 9; row++)
        {
            for (int col = 0; col < 9; col++)
            {
                if (board.cells[row][col] != 0) continue;
                if (grid.hasCandidate(row, col, digit))
                {
                    candidateColsPerRow[row].add(col);
                }
            }
        }

        for (int rowA = 0; rowA < 9; rowA++)
        {
            if (candidateColsPerRow[rowA].size != 2) continue;

            for (int rowB = rowA + 1; rowB < 9; rowB++)
            {
                if (!candidateColsPerRow[rowB].samePair(candidateColsPerRow[rowA])) continue;

                // rowA and rowB both confine `digit` to the same two columns.
                int colX = candidateColsPerRow[rowA].values[0];
                int colY = candidateColsPerRow[rowA].values[1];
                TraceEvent event;
                event.technique = TechniqueType::XWingRow;

                for (int row = 0; row < 9; row++)
                {
                    if (row == rowA || row == rowB) continue;
                    for (int col : {colX, colY})
                    {
                        if (board.cells[row][col] != 0) continue;
                        if (grid.hasCandidate(row, col, digit))
                        {
                            grid.eliminate(row, col, digit);
                            event.eliminations.push_back({row, col, digit});
                        }
                    }
                }

                if (!event.eliminations.empty()) return event;
            }
        }
    }
    return std::nullopt;
}

std::optional<TraceEvent> Heuristics::xWingCols(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored)
{
    (void)nodesExplored; // this technique only eliminates, never places

    for (int digit = 1; digit <= 9; digit++)
    {
        std::array<PairSlots, 9> candidateRowsPerCol{};
        for (int col = 0; col < 9; col++)
        {
            for (int row = 0; row < 9; row++)
            {
                if (board.cells[row][col] != 0) continue;
                if (grid.hasCandidate(row, col, digit))
                {
                    candidateRowsPerCol[col].add(row);
                }
            }
        }

        for (int colA = 0; colA < 9; colA++)
        {
            if (candidateRowsPerCol[colA].size != 2) continue;

            for (int colB = colA + 1; colB < 9; colB++)
            {
                if (!candidateRowsPerCol[colB].samePair(candidateRowsPerCol[colA])) continue;

                int rowX = candidateRowsPerCol[colA].values[0];
                int rowY = candidateRowsPerCol[colA].values[1];
                TraceEvent event;
                event.technique = TechniqueType::XWingCol;

                for (int col = 0; col < 9; col++)
                {
                    if (col == colA || col == colB) continue;
                    for (int row : {rowX, rowY})
                    {
                        if (board.cells[row][col] != 0) continue;
                        if (grid.hasCandidate(row, col, digit))
                        {
                            grid.eliminate(row, col, digit);
                            event.eliminations.push_back({row, col, digit});
                        }
                    }
                }

                if (!event.eliminations.empty()) return event;
            }
        }
    }
    return std::nullopt;
}
```

### src/new stuff/xWing.hpp

```cpp
#pragma once
#include "../board.hpp"
#include "../candidateGrid.hpp"
#include "../trace/traceEvent.hpp"

namespace Heuristics
{
    // Row-based X-Wing: a digit is confined to the same two columns in
    // two different rows -- eliminate it from those two columns in every
    // other row. Places nothing; nodesExplored is unused but kept so
    // every technique shares one signature.
    std::optional<TraceEvent> xWingRows(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);

    // Column-based X-Wing: the same idea with rows and columns swapped.
    std::optional<TraceEvent> xWingCols(Board& board, CandidateGrid& grid, std::uint64_t& nodesExplored);
}
```

### src/solver1.hpp

```cpp
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
```

### src/solver2.hpp

```cpp
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
```

### src/solver3.cpp

```cpp
#include "solver3.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult Solver3::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair}).solve(board, captureTrace, statistics);
}

SolveResult Solver3::solveWithFirstEmptyCell(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair},
                           CellSelection::findFirstEmptyCell).solve(board, captureTrace, statistics);
}
```

### src/solver3.hpp

```cpp
#pragma once
#include "new stuff/HeuristicEngine.hpp"

class Solver3
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
    // Same propagation set as Solver3, but first-empty branching. This is
    // the control run for isolating MRV's effect from the techniques'.
    static SolveResult solveWithFirstEmptyCell(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
```

### src/solver4.cpp

```cpp
#include "solver4.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedPair.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"

SolveResult Solver4::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair, Heuristics::nakedPair}).solve(board, captureTrace, statistics);
}
```

### src/solver4.hpp

```cpp
#pragma once
#include "new stuff/HeuristicEngine.hpp"

class Solver4
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
```

### src/solver5.cpp

```cpp
#include "solver5.hpp"
#include "new stuff/hiddenSingle.hpp"
#include "new stuff/nakedPair.hpp"
#include "new stuff/nakedSingle.hpp"
#include "new stuff/pointingPair.hpp"
#include "new stuff/xWing.hpp"

SolveResult Solver5::solve(const Board& board, bool captureTrace, SolveStatistics* statistics)
{
    return BitmaskHeuristicEngine({Heuristics::nakedSingle, Heuristics::hiddenSingle,
                            Heuristics::pointingPair, Heuristics::nakedPair,
                            Heuristics::xWingRows, Heuristics::xWingCols}).solve(board, captureTrace, statistics);
}
```

### src/solver5.hpp

```cpp
#pragma once
#include "new stuff/HeuristicEngine.hpp"

class Solver5
{
public:
    static SolveResult solve(const Board& board, bool captureTrace = true, SolveStatistics* statistics = nullptr);
};
```

### src/solveResult.hpp

```cpp
#pragma once
#include "board.hpp"
#include "trace/traceEvent.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

// The same totals as summarizing a full trace, without retaining its events.
struct SolveStatistics
{
    int maxDepth = 0;
    std::uint64_t guesses = 0;
    std::uint64_t deadEnds = 0;
    std::uint64_t placements = 0;
    std::uint64_t eliminations = 0;
    std::uint64_t events = 0;

    void recordEvent(const TraceEvent& event)
    {
        maxDepth = std::max(maxDepth, event.depth);
        guesses += event.isGuess;
        placements += event.placement.has_value();
        eliminations += event.eliminations.size();
        ++events;
        // A guess becomes a dead end later, when its branch fails.
    }
};

// Every solver returns this same struct. nodesExplored is universal: it counts
// successful candidate placements that enter a search branch.  Rejected
// legality checks and deterministic deductions are reported separately in the
// trace, so a node means the same thing for every solver implementation.
struct SolveResult
{
    bool solved = false;
    std::uint64_t nodesExplored = 0;
    double timeMs = 0.0;
    // Retained with the final board so a generated trace can replay the solve
    // without reading the dataset again.
    Board startingBoard;
    Board board;
    std::vector<TraceEvent> trace;
};
```

### src/trace/jsonWriter.cpp

```cpp
#include "jsonWriter.hpp"

#include <iomanip>
#include <sstream>

namespace
{
void writeString(std::ostream& out, const std::string& value)
{
    out << '"';
    for (unsigned char character : value)
    {
        switch (character)
        {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        case '<': out << "\\u003c"; break; // avoid ending the script element
        default: out << character; break;
        }
    }
    out << '"';
}

void writeCellDigit(std::ostream& out, const CellDigit& cell)
{
    out << "{\"row\":" << cell.row
        << ",\"col\":" << cell.col
        << ",\"digit\":" << cell.digit << '}';
}
}

std::string TraceJson::toJson(const SolveResult& result, const std::string& puzzleId)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(6);
    out << '{';
    out << "\"puzzleId\":";
    writeString(out, puzzleId);
    out << ",\"solved\":" << (result.solved ? "true" : "false")
        << ",\"nodesExplored\":" << result.nodesExplored
        << ",\"timeMs\":" << result.timeMs
        << ",\"startingBoard\":";
    writeString(out, result.startingBoard.toString());
    out << ",\"solvedBoard\":";
    writeString(out, result.board.toString());
    out << ",\"trace\":[";

    for (std::size_t index = 0; index < result.trace.size(); ++index)
    {
        if (index != 0) out << ',';
        const TraceEvent& event = result.trace[index];
        out << "{\"id\":" << event.id
            << ",\"parentId\":" << event.parentId
            << ",\"depth\":" << event.depth
            << ",\"technique\":";
        writeString(out, techniqueName(event.technique));
        out << ",\"placement\":";
        if (event.placement) writeCellDigit(out, *event.placement);
        else out << "null";
        out << ",\"eliminations\":[";
        for (std::size_t elimination = 0; elimination < event.eliminations.size(); ++elimination)
        {
            if (elimination != 0) out << ',';
            writeCellDigit(out, event.eliminations[elimination]);
        }
        out << "]"
            << ",\"mrvSnapshot\":[";
        for (std::size_t snapshot = 0; snapshot < event.mrvSnapshot.size(); ++snapshot)
        {
            if (snapshot != 0) out << ',';
            const MrvCellCount& cell = event.mrvSnapshot[snapshot];
            out << "{\"row\":" << cell.row << ",\"col\":" << cell.col
                << ",\"count\":" << cell.count << '}';
        }
        out << "]"
            << ",\"isGuess\":" << (event.isGuess ? "true" : "false")
            << ",\"leadsToConflict\":" << (event.leadsToConflict ? "true" : "false")
            << ",\"restoredBoard\":";
        writeString(out, event.restoredBoard);
        out
            << ",\"valid\":" << (event.valid ? "true" : "false")
            << '}';
    }

    out << "]}";
    return out.str();
}
```

### src/trace/jsonWriter.hpp

```cpp
#pragma once

#include "../solveResult.hpp"
#include <string>

namespace TraceJson
{
    // Produces the complete, self-contained data payload embedded by the
    // visualizer.  No external JSON dependency is needed for this numeric data.
    std::string toJson(const SolveResult& result, const std::string& puzzleId);
}
```

### src/trace/traceEvent.hpp

```cpp
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// A technique name is deliberately stored as an enum in C++.  The JSON writer
// converts it to a stable, human-readable string for the browser.
enum class TechniqueType
{
    NakedSingle,
    HiddenSingle,
    NakedPair,
    PointingPair,
    XWingRow,
    XWingCol,
    Guess,
    // Emitted after a failed branch has been restored.  This is separate
    // from the guess so a trace can replay the real search timeline rather
    // than only the final, successful route.
    Backtrack,
    RawAttempt
};

inline const char* techniqueName(TechniqueType technique)
{
    switch (technique)
    {
    case TechniqueType::NakedSingle: return "Naked single";
    case TechniqueType::HiddenSingle: return "Hidden single";
    case TechniqueType::NakedPair: return "Naked pair";
    case TechniqueType::PointingPair: return "Pointing pair";
    case TechniqueType::XWingRow: return "X-Wing (row)";
    case TechniqueType::XWingCol: return "X-Wing (column)";
    case TechniqueType::Guess: return "Guess";
    case TechniqueType::Backtrack: return "Backtrack";
    case TechniqueType::RawAttempt: return "Raw attempt";
    }
    return "Unknown";
}

struct CellDigit
{
    int row = 0;
    int col = 0;
    int digit = 0;
};

// Candidate counts considered by the cell selector immediately before a
// branching decision.  Keeping the whole comparison makes MRV observable,
// rather than showing only its eventual winner.
struct MrvCellCount
{
    int row = 0;
    int col = 0;
    int count = 0;
};

struct TraceEvent
{
    std::uint64_t id = 0;
    std::uint64_t parentId = 0;
    int depth = 0; // Guess-branch depth, never a propagation-step count.
    TechniqueType technique = TechniqueType::RawAttempt;
    std::optional<CellDigit> placement;
    std::vector<CellDigit> eliminations;
    std::vector<MrvCellCount> mrvSnapshot;
    bool isGuess = false;
    bool leadsToConflict = false;
    // A backtrack restores the complete board snapshot that existed before
    // its failed guess.  Other events leave this empty because their board
    // change is already described by placement/eliminations.
    std::string restoredBoard;
    // Raw attempts use this to distinguish rejected legality checks from
    // placements that actually entered the search tree.
    bool valid = true;
};
```

### src/treeLogger.hpp

```cpp
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
```

### src/treeTraversalVisualizer.cpp

```cpp
#include "treeTraversalVisualizer.hpp"

#include "trace/jsonWriter.hpp"

#include <fstream>
#include <iomanip>
#include <map>

namespace
{
struct TraceStats { std::size_t events = 0, guesses = 0, failedGuesses = 0, placements = 0, eliminations = 0; std::map<std::string, std::size_t> techniques; };

TraceStats summarize(const SolveResult& result)
{
    TraceStats stats;
    stats.events = result.trace.size();
    for (const TraceEvent& event : result.trace)
    {
        ++stats.techniques[techniqueName(event.technique)];
        stats.guesses += event.isGuess;
        stats.failedGuesses += event.isGuess && event.leadsToConflict;
        stats.placements += event.placement.has_value();
        stats.eliminations += event.eliminations.size();
    }
    return stats;
}

std::string escapeHtml(const std::string& text)
{
    std::string result;
    for (char character : text)
    {
        if (character == '&') result += "&amp;";
        else if (character == '<') result += "&lt;";
        else if (character == '>') result += "&gt;";
        else if (character == '"') result += "&quot;";
        else result += character;
    }
    return result;
}
}

bool TreeTraversalVisualizer::writeHtml(const SolveResult& result, const std::string& puzzleId,
                                        const std::string& htmlPath)
{
    std::ofstream out(htmlPath);
    if (!out) return false;
    const TraceStats stats = summarize(result);

    out << R"HTML(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Sudoku solver trace</title>
<style>
:root{color-scheme:dark;font-family:system-ui,-apple-system,"Segoe UI",sans-serif;background:#101821;color:#f4f8fb}*{box-sizing:border-box}body{margin:0}.page{max-width:1500px;margin:auto;padding:20px}header{display:flex;justify-content:space-between;gap:18px;align-items:flex-start;flex-wrap:wrap;margin-bottom:16px}h1,h2,h3{margin:0}.subtitle,.hint,.detail,.step{color:#c2d0d8}.metrics,.legend,.controls,.filters{display:flex;gap:8px;flex-wrap:wrap}.metric,.analysis-card,.event-card{padding:10px;background:#182a36;border:1px solid #486171;border-radius:7px}.metric b{display:block;font-size:1.05rem}.tabs{display:flex;gap:6px;flex-wrap:wrap;border-bottom:1px solid #456070;margin-bottom:14px}.tab,.control,.filter{font:inherit;color:#edf5f8;background:#1b303d;border:1px solid #4e6979;border-radius:5px;padding:8px 11px;cursor:pointer}.tab{border-radius:5px 5px 0 0;border-bottom:0}.tab[aria-selected="true"],.filter[aria-pressed="true"]{background:#d93645;color:#fff;border-color:#ff9ba5}.panel{display:none}.panel.active{display:block}.panel-head{display:flex;justify-content:space-between;gap:12px;align-items:center;flex-wrap:wrap;margin-bottom:10px}.tree-wrap{overflow:auto;background:#14212b;border:1px solid #3d5768;border-radius:8px}.tree{display:block;min-width:700px}.edge{stroke:#6e8797;stroke-width:2}.tree-node{cursor:pointer}.node-shape{stroke:#edf7fb;stroke-width:1.5}.node-label{fill:#fff;font-size:10px;text-anchor:middle;pointer-events:none}.node-sub{fill:#c8d8df;font-size:8px;text-anchor:middle;pointer-events:none}.NakedSingle{fill:#257e67}.HiddenSingle{fill:#337ebd}.NakedPair{fill:#805ac0}.PointingPair{fill:#b76c39}.XWingrow,.XWingcolumn{fill:#b38a2c}.Guess{fill:#6850a8}.Backtrack{fill:#c85422}.RawAttempt{fill:#617787}.conflict .node-shape,.invalid .node-shape{fill:#a72935;stroke:#ffd2d7}.solution .node-shape{stroke:#ffeff0;stroke-width:3}.root-shape{fill:#273e4c;stroke:#e8f3f7}.root-label{fill:#fff;font-size:11px;text-anchor:middle}.legend{margin:10px 0}.legend span{font-size:.83rem}.swatch{display:inline-block;width:12px;height:12px;border-radius:2px;margin-right:4px;vertical-align:-1px}.red{background:#ef3e4d}.purple{background:#6850a8}.orange{background:#b76c39}.gray{background:#617787}.mrv-layout,.player-layout,.analysis-layout{display:grid;grid-template-columns:minmax(230px,1fr) minmax(320px,2fr);gap:18px}.mrv-select,.speed{width:100%;padding:8px;background:#192c38;color:#fff;border:1px solid #4d6979;border-radius:5px}.fan{display:grid;grid-template-columns:repeat(auto-fill,minmax(67px,1fr));gap:7px}.fan-cell{min-height:72px;padding:5px;background:#1d303d;text-align:center;border-radius:4px}.fan-cell.chosen{outline:2px solid #ff5d69;background:#54232a}.bar-track{height:32px;background:#11202a;display:flex;align-items:end;justify-content:center;margin:4px 0}.bar{width:23px;background:#4e9fd1}.chosen .bar{background:#ff5060}.board{display:grid;grid-template-columns:repeat(9,1fr);border:2px solid #d7e3e8;background:#d7e3e8;gap:1px;width:min(100%,520px);aspect-ratio:1}.cell{position:relative;display:grid;place-items:center;background:#172832;min-width:0}.cell:nth-child(3n){border-right:2px solid #d7e3e8}.cell:nth-child(n+19):nth-child(-n+27),.cell:nth-child(n+46):nth-child(-n+54){border-bottom:2px solid #d7e3e8}.cell.current{background:#592126;box-shadow:inset 0 0 0 3px #ff6370}.cell.affected{background:#3e2528}.digit{font-size:clamp(1rem,3vw,1.8rem);font-weight:700}.given .digit{color:#edf5f8}.placed .digit{color:#ff5a67;text-shadow:0 0 8px #d93645}.candidates{display:grid;grid-template-columns:repeat(3,1fr);grid-template-rows:repeat(3,1fr);width:100%;height:100%;padding:5px;color:#b6cad4;font-size:clamp(.45rem,1.2vw,.8rem);text-align:center}.candidate{display:grid;place-items:center}.event-card{min-height:116px;margin-top:10px}.analysis-card{overflow:auto}.event-table{width:100%;border-collapse:collapse;font-size:.87rem}.event-table th,.event-table td{padding:7px;text-align:left;border-bottom:1px solid #365161;vertical-align:top}.event-table th,th{color:#ffd5d9}.event-table tr.route td:first-child{color:#ff7781;font-weight:700}.event-table tr.dead td{color:#ffb3ba}.export{margin-left:auto}table{width:100%;border-collapse:collapse}th,td{padding:7px;border-bottom:1px solid #365161;text-align:left}.status{font-weight:700;color:#ff7c85}@media(max-width:760px){.page{padding:12px}.mrv-layout,.player-layout,.analysis-layout{grid-template-columns:1fr}.tree{min-width:620px}.cell:nth-child(3n){border-right:1px solid #d7e3e8}}
</style></head><body><main class="page">
<header><div><h1>Sudoku solver trace</h1><p class="subtitle" id="subtitle"></p></div><div class="metrics" id="metrics"></div></header>
<nav class="tabs" aria-label="Trace views"><button class="tab" type="button" data-view="tree" aria-selected="true">Branching tree</button><button class="tab" type="button" data-view="mrv" aria-selected="false">MRV fan-out</button><button class="tab" type="button" data-view="player" aria-selected="false">Board player</button><button class="tab" type="button" data-view="analysis" aria-selected="false">Analysis</button></nav>
<section class="panel active" id="tree-panel"><div class="panel-head"><h2>Placements and eliminations</h2><div class="filters" id="filters"></div></div><div class="legend"><span><i class="swatch red"></i>solution route</span><span><i class="swatch purple"></i>guess</span><span><i class="swatch orange"></i>elimination</span><span><i class="swatch gray"></i>raw attempt</span></div><div class="detail" id="tree-detail" aria-live="polite"></div><div class="tree-wrap"><svg class="tree" id="tree" role="img" aria-label="Solver trace tree"></svg></div></section>
<section class="panel" id="mrv-panel"><div class="panel-head"><h2>MRV candidate comparison</h2><span class="hint">Every empty cell considered before a guess.</span></div><div class="mrv-layout"><div><label for="mrv-select">Guess point</label><select id="mrv-select" class="mrv-select"></select><p class="detail" id="mrv-detail"></p></div><div class="analysis-card"><div class="fan" id="fan"></div></div></div></section>
<section class="panel" id="player-panel"><div class="panel-head"><h2>Watch the solution route</h2><span class="hint">Solution placements are red. Space = play/pause; ←/→ = step; Home = reset.</span></div><div class="player-layout"><div class="board" id="board" role="img" aria-label="Sudoku board replay"></div><div><div class="controls"><button class="control" type="button" id="previous">Previous</button><button class="control" type="button" id="play">Play</button><button class="control" type="button" id="next">Next</button><button class="control" type="button" id="reset">Reset</button></div><label for="speed">Playback speed</label><select id="speed" class="speed"><option value="800">Slow</option><option value="450" selected>Normal</option><option value="180">Fast</option></select><input id="scrub" type="range" min="0" value="0" aria-label="Trace step" style="width:100%;margin-top:12px;accent-color:#ef3e4d"><p class="step" id="step-label" aria-live="polite"></p><div class="event-card" id="event-card"></div></div></div></section>
<section class="panel" id="analysis-panel"><div class="panel-head"><h2>Trace analysis</h2><button class="control export" type="button" id="export">Download trace CSV</button></div><div class="analysis-layout"><div class="analysis-card"><h3>Technique counts</h3><table><thead><tr><th>Technique</th><th>Events</th></tr></thead><tbody>)HTML";
    for (const auto& [name, count] : stats.techniques)
        out << "<tr><td>" << escapeHtml(name) << "</td><td>" << count << "</td></tr>";
    out << R"HTML(</tbody></table></div><div class="analysis-card"><h3>Recorded events</h3><table class="event-table"><thead><tr><th>Step</th><th>Technique</th><th>Action</th><th>Depth</th><th>Outcome</th></tr></thead><tbody id="event-rows"></tbody></table></div></div></section>
</main><script id="trace-data" type="application/json">)HTML";
    out << TraceJson::toJson(result, puzzleId);
    out << R"HTML(</script><script>
(()=>{const data=JSON.parse(document.getElementById('trace-data').textContent),trace=data.trace||[],$=id=>document.getElementById(id),byId=new Map(trace.map((e,i)=>[e.id,{...e,index:i}]));const safe=v=>String(v),classes={'Naked single':'NakedSingle','Hidden single':'HiddenSingle','Naked pair':'NakedPair','Pointing pair':'PointingPair','X-Wing (row)':'XWingrow','X-Wing (column)':'XWingcolumn','Guess':'Guess','Backtrack':'Backtrack','Raw attempt':'RawAttempt'},tech=e=>classes[e.technique]||'RawAttempt';const failed=e=>{let p=e,s=new Set;while(p&&p.parentId&&!s.has(p.id)){if(p.isGuess&&p.leadsToConflict)return true;s.add(p.id);p=byId.get(p.parentId)}return !!(e&&e.isGuess&&e.leadsToConflict)},route=trace.filter(e=>!failed(e)),routeIds=new Set(route.map(e=>e.id));let enabled=new Set(trace.map(e=>e.technique)),step=0,timer=null;
$('subtitle').textContent=`${data.puzzleId} · ${data.solved?'Solved':'Unsolved'} · ${trace.length} recorded events`;const counts={guesses:trace.filter(e=>e.isGuess).length,failed:trace.filter(e=>e.isGuess&&e.leadsToConflict).length,placements:trace.filter(e=>e.placement).length,eliminations:trace.reduce((n,e)=>n+(e.eliminations||[]).length,0)};[['Node metric',data.nodesExplored],['Solve time',Number(data.timeMs).toFixed(3)+' ms'],['Guesses',counts.guesses],['Failed guesses',counts.failed],['Placements',counts.placements],['Eliminations',counts.eliminations]].forEach(([k,v])=>$('metrics').insertAdjacentHTML('beforeend',`<div class="metric"><b>${safe(v)}</b>${safe(k)}</div>`));document.querySelectorAll('.tab').forEach(t=>t.onclick=()=>{document.querySelectorAll('.tab').forEach(b=>b.setAttribute('aria-selected',String(b===t)));document.querySelectorAll('.panel').forEach(p=>p.classList.toggle('active',p.id===t.dataset.view+'-panel'))});
const action=e=>{if(e.technique==='Backtrack')return `backtrack from r${e.placement.row+1}c${e.placement.col+1} = ${e.placement.digit}`;const p=e.placement?`r${e.placement.row+1}c${e.placement.col+1} = ${e.placement.digit}`:'',x=(e.eliminations||[]).map(c=>`${c.digit} from r${c.row+1}c${c.col+1}`).join(', ');return[p,x&&'remove '+x].filter(Boolean).join('; ')||'No board change'},outcome=e=>e.technique==='Backtrack'?'Returned after dead end':e.valid===false?'Rejected':e.leadsToConflict?'Dead end':routeIds.has(e.id)?'Solution route':'Branch event';
function filters(){const h=$('filters');h.replaceChildren();[...new Set(trace.map(e=>e.technique))].forEach(n=>{const b=document.createElement('button');b.className='filter';b.type='button';b.textContent=n;b.setAttribute('aria-pressed',String(enabled.has(n)));b.onclick=()=>{enabled.has(n)?enabled.delete(n):enabled.add(n);filters();tree()};h.append(b)})}
function tree(){const svg=$('tree'),ns='http://www.w3.org/2000/svg';svg.replaceChildren();const es=trace.filter(e=>enabled.has(e.technique)&&(e.placement||(e.eliminations||[]).length));if(!es.length){$('tree-detail').textContent='No visible events match the selected filters.';return}const nodes=new Map([['root',{key:'root',children:[],level:0}]]),tails=new Map;es.forEach(e=>{let p=(e.isGuess||e.technique==='Raw attempt')?e.parentId:(tails.get(e.parentId)||e.parentId);p=p&&nodes.has(p)?p:'root';const n={key:e.id,event:e,parent:p,children:[],x:0,level:0};nodes.set(e.id,n);nodes.get(p).children.push(n);if(e.isGuess)tails.set(e.id,e.id);else if(e.technique!=='Raw attempt')tails.set(e.parentId,e.id)});let leaf=70,max=0;const pos=(n,l)=>{n.level=l;max=Math.max(max,l);if(!n.children.length)return n.x=leaf,leaf+=115,n.x;const xs=n.children.map(c=>pos(c,l+1));return n.x=xs.reduce((a,b)=>a+b)/xs.length,n.x};pos(nodes.get('root'),0);const w=Math.max(720,leaf+60),h=Math.max(260,(max+1)*86+65);svg.setAttribute('viewBox',`0 0 ${w} ${h}`);svg.setAttribute('width',w);svg.setAttribute('height',h);const make=x=>document.createElementNS(ns,x);[...nodes.values()].filter(n=>n.key!=='root').forEach(n=>{const p=nodes.get(n.parent),l=make('line');l.setAttribute('class','edge');l.setAttribute('x1',p.x);l.setAttribute('y1',p.level*86+44);l.setAttribute('x2',n.x);l.setAttribute('y2',n.level*86+32);svg.append(l)});[...nodes.values()].forEach(n=>{const g=make('g');g.setAttribute('transform',`translate(${n.x},${n.level*86+25})`);if(n.key==='root'){const c=make('circle');c.setAttribute('r',21);c.setAttribute('class','root-shape');g.append(c);const t=make('text');t.setAttribute('class','root-label');t.setAttribute('y',4);t.textContent='Start';g.append(t);svg.append(g);return}const e=n.event;g.setAttribute('class',`tree-node ${tech(e)}${e.leadsToConflict?' conflict':''}${e.valid===false?' invalid':''}${routeIds.has(e.id)?' solution':''}`);const shape=make(e.placement?'circle':'path');if(e.placement)shape.setAttribute('r',e.isGuess?22:19);else shape.setAttribute('d','M0,-17 L17,0 L0,17 L-17,0 Z');shape.setAttribute('class','node-shape');g.append(shape);const label=make('text');label.setAttribute('class','node-label');label.setAttribute('y',-2);label.textContent=e.placement?`${e.placement.digit} @ r${e.placement.row+1}c${e.placement.col+1}`:`−${e.eliminations.length}`;g.append(label);const sub=make('text');sub.setAttribute('class','node-sub');sub.setAttribute('y',10);sub.textContent=e.technique;g.append(sub);g.onclick=()=>setPlayer(e.id);svg.append(g)});$('tree-detail').textContent=`${es.length} visible events. Red outlines identify the final solution route.`}
function mrv(){const gs=trace.filter(e=>e.isGuess&&(e.mrvSnapshot||[]).length),s=$('mrv-select');s.replaceChildren();if(!gs.length){s.disabled=true;$('mrv-detail').textContent='No guesses were needed.';return}gs.forEach((e,i)=>s.add(new Option(`Guess ${i+1}: r${e.placement.row+1}c${e.placement.col+1} = ${e.placement.digit}`,i)));const draw=()=>{const e=gs[+s.value],max=Math.max(...e.mrvSnapshot.map(c=>c.count),1),fan=$('fan');fan.replaceChildren();e.mrvSnapshot.forEach(c=>{const chosen=c.row===e.placement.row&&c.col===e.placement.col,d=document.createElement('div');d.className='fan-cell'+(chosen?' chosen':'');d.innerHTML=`r${c.row+1}c${c.col+1}<div class="bar-track"><div class="bar" style="height:${Math.max(5,c.count/max*100)}%"></div></div>${c.count} candidates`;fan.append(d)});$('mrv-detail').textContent='Selected MRV cell is red.'};s.onchange=draw;draw()}
const index=(r,c)=>r*9+c,peers=(r,c)=>{const s=new Set;for(let i=0;i<9;i++){s.add(index(r,i));s.add(index(i,c))}for(let y=Math.floor(r/3)*3;y<Math.floor(r/3)*3+3;y++)for(let x=Math.floor(c/3)*3;x<Math.floor(c/3)*3+3;x++)s.add(index(y,x));s.delete(index(r,c));return s};function state(n){const board=data.startingBoard.split('').map(v=>v==='0'||v==='.'?0:+v),cand=Array.from({length:81},(_,i)=>board[i]?new Set:new Set([1,2,3,4,5,6,7,8,9])),put=p=>{board[index(p.row,p.col)]=p.digit;cand[index(p.row,p.col)].clear();peers(p.row,p.col).forEach(i=>cand[i].delete(p.digit))};board.forEach((d,i)=>{if(d)peers(Math.floor(i/9),i%9).forEach(j=>cand[j].delete(d))});for(let i=0;i<n;i++){const e=route[i];if(e.placement&&(e.technique!=='Raw attempt'||e.valid))put(e.placement);(e.eliminations||[]).forEach(c=>cand[index(c.row,c.col)].delete(c.digit))}return{board,cand}}
function player(){const s=state(step),cur=step?route[step-1]:null,affected=new Set((cur?.eliminations||[]).map(c=>index(c.row,c.col))),b=$('board');b.replaceChildren();s.board.forEach((d,i)=>{const cell=document.createElement('div'),given=data.startingBoard[i]!=='0'&&data.startingBoard[i]!=='.';cell.className='cell'+(d?(given?' given':' placed'):'')+(cur?.placement&&index(cur.placement.row,cur.placement.col)===i?' current':'')+(affected.has(i)?' affected':'');if(d){const z=document.createElement('span');z.className='digit';z.textContent=d;cell.append(z)}else{const m=document.createElement('div');m.className='candidates';for(let k=1;k<=9;k++){const q=document.createElement('span');q.className='candidate';q.textContent=s.cand[i].has(k)?k:'';m.append(q)}cell.append(m)}b.append(cell)});$('scrub').value=step;$('step-label').textContent=`Step ${step} of ${route.length} solution-route events`;$('event-card').innerHTML=cur?`<h3>${safe(cur.technique)}</h3><p>${safe(action(cur))}</p><p class="status">${safe(outcome(cur))}</p>`:'<h3>Starting board</h3><p>Original clues are neutral; solution-route placements are high-contrast red.</p>'}
function stop(){if(timer)clearInterval(timer);timer=null;$('play').textContent='Play'}function setPlayer(id){const i=route.findIndex(e=>e.id===id);if(i>=0){stop();step=i+1;player();document.querySelector('[data-view="player"]').click()}}function next(){stop();step=Math.min(route.length,step+1);player()}function previous(){stop();step=Math.max(0,step-1);player()}$('scrub').max=route.length;$('scrub').oninput=e=>{stop();step=+e.target.value;player()};$('next').onclick=next;$('previous').onclick=previous;$('reset').onclick=()=>{stop();step=0;player()};$('play').onclick=()=>{if(timer)return stop();if(step>=route.length)step=0;$('play').textContent='Pause';timer=setInterval(()=>{step++;player();if(step>=route.length)stop()},+$('speed').value)};$('speed').onchange=()=>{if(timer){stop();$('play').click()}};document.addEventListener('keydown',e=>{if(e.target.tagName==='SELECT')return;if(e.code==='Space'){e.preventDefault();$('play').click()}else if(e.key==='ArrowRight')next();else if(e.key==='ArrowLeft')previous();else if(e.key==='Home')$('reset').click()});
function analysis(){const b=$('event-rows');trace.forEach((e,i)=>{const r=document.createElement('tr');r.className=(routeIds.has(e.id)?'route ':'')+(e.leadsToConflict?'dead':'');r.innerHTML=`<td>${i+1}</td><td>${safe(e.technique)}</td><td>${safe(action(e))}</td><td>${e.depth}</td><td>${safe(outcome(e))}</td>`;b.append(r)})}$('export').onclick=()=>{const rows=[['step','technique','placement','eliminations','depth','outcome'],...trace.map((e,i)=>[i+1,e.technique,e.placement?`r${e.placement.row+1}c${e.placement.col+1}=${e.placement.digit}`:'',(e.eliminations||[]).map(c=>`${c.digit}@r${c.row+1}c${c.col+1}`).join('; '),e.depth,outcome(e)])],csv=rows.map(r=>r.map(v=>'"'+String(v).replaceAll('"','""')+'"').join(',')).join('\n'),a=document.createElement('a');a.href=URL.createObjectURL(new Blob([csv],{type:'text/csv'}));a.download='sudoku-trace.csv';a.click();URL.revokeObjectURL(a.href)};filters();tree();mrv();player();analysis();})();
</script></body></html>)HTML";
    return static_cast<bool>(out);
}

bool TreeTraversalVisualizer::writeRuntimeHtml(const SolveResult& result, const std::string& puzzleId,
                                               const std::string& htmlPath)
{
    std::ofstream out(htmlPath);
    if (!out) return false;

    out << R"HTML(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Sudoku full runtime trace</title>
<style>
:root{color-scheme:dark;font-family:system-ui,-apple-system,"Segoe UI",sans-serif;background:#101821;color:#f4f8fb}*{box-sizing:border-box}body{margin:0}.page{max-width:1500px;margin:auto;padding:20px}header{display:flex;justify-content:space-between;gap:14px;align-items:flex-start;flex-wrap:wrap;margin-bottom:16px}h1,h2,h3,p{margin-top:0}.subtitle,.hint,.detail{color:#c2d0d8}.metrics{display:flex;gap:8px;flex-wrap:wrap}.metric,.card{padding:10px;background:#182a36;border:1px solid #486171;border-radius:7px}.metric b{display:block;font-size:1.05rem}.layout{display:grid;grid-template-columns:minmax(310px,1fr) minmax(320px,1.3fr);gap:18px}.board{display:grid;grid-template-columns:repeat(9,1fr);border:2px solid #d7e3e8;background:#d7e3e8;gap:1px;width:min(100%,560px);aspect-ratio:1}.cell{position:relative;display:grid;place-items:center;background:#172832;min-width:0}.cell:nth-child(3n){border-right:2px solid #d7e3e8}.cell:nth-child(n+19):nth-child(-n+27),.cell:nth-child(n+46):nth-child(-n+54){border-bottom:2px solid #d7e3e8}.digit{font-size:clamp(1rem,3vw,1.9rem);font-weight:700}.given{color:#edf5f8}.placed{color:#65d8b7}.current{background:#50411e;box-shadow:inset 0 0 0 3px #ffe16a}.current.wrong{background:#62242b;box-shadow:inset 0 0 0 4px #ff5c68}.current.backtrack{background:#65351f;box-shadow:inset 0 0 0 4px #ff9b54}.controls{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:12px}.control{font:inherit;color:#edf5f8;background:#1b303d;border:1px solid #4e6979;border-radius:5px;padding:8px 11px;cursor:pointer}.control:hover{background:#274858}.scrub{width:100%;accent-color:#ff5c68}.event{min-height:118px}.status{font-weight:700}.wrong-text{color:#ff8892}.backtrack-text{color:#ffb270}.timeline-title{display:flex;justify-content:space-between;gap:12px;align-items:center;margin:18px 0 8px}.timeline{display:grid;grid-template-columns:repeat(auto-fill,minmax(170px,1fr));gap:7px;max-height:420px;overflow:auto;padding:3px}.event-step{font:inherit;text-align:left;color:#e7f2f5;background:#182a36;border:1px solid #486171;border-radius:5px;padding:8px;cursor:pointer}.event-step:hover,.event-step.selected{border-color:#ffe16a;background:#314653}.event-step.wrong{border-color:#ff5c68;background:#54242b}.event-step.backtrack{border-color:#ff9b54;background:#553323}.event-step .num{display:block;color:#a9c0ca;font-size:.78rem}.legend{display:flex;gap:13px;flex-wrap:wrap;color:#c2d0d8;font-size:.9rem}.swatch{display:inline-block;width:12px;height:12px;border-radius:2px;margin-right:4px;vertical-align:-1px}.yellow{background:#ffe16a}.red{background:#ff5c68}.orange{background:#ff9b54}.green{background:#65d8b7}@media(max-width:760px){.page{padding:12px}.layout{grid-template-columns:1fr}.cell:nth-child(3n){border-right:1px solid #d7e3e8}}
</style></head><body><main class="page"><header><div><h1>Full solver runtime trace</h1><p class="subtitle" id="subtitle"></p><div class="legend"><span><i class="swatch yellow"></i>current event</span><span><i class="swatch red"></i>wrong guess / dead end</span><span><i class="swatch orange"></i>backtrack restore</span><span><i class="swatch green"></i>successful placement</span></div></div><div class="metrics" id="metrics"></div></header><section class="layout"><div class="board" id="board" aria-label="Sudoku board at selected runtime step"></div><div class="card"><div class="controls"><button class="control" id="previous" type="button">Previous</button><button class="control" id="play" type="button">Play</button><button class="control" id="next" type="button">Next</button><button class="control" id="reset" type="button">Reset</button></div><label for="scrub">Runtime position</label><input class="scrub" id="scrub" type="range" min="0" value="0"><p class="detail" id="step-label"></p><div class="card event" id="event-card"></div></div></section><div class="timeline-title"><h2>Every recorded event</h2><span class="hint">Click a step, or play through the exact search order.</span></div><section class="timeline" id="timeline"></section></main><script id="trace-data" type="application/json">)HTML";
    out << TraceJson::toJson(result, puzzleId);
    out << R"HTML(</script><script>
(()=>{const data=JSON.parse(document.getElementById('trace-data').textContent),trace=data.trace||[],$=id=>document.getElementById(id),safe=v=>String(v),idx=(r,c)=>r*9+c,fromText=text=>text.split('').map(v=>v==='0'||v==='.'?0:+v),isBacktrack=e=>e.technique==='Backtrack',isWrong=e=>e.isGuess&&e.leadsToConflict;let step=0,timer=null;
const counts={guesses:trace.filter(e=>e.isGuess).length,wrong:trace.filter(isWrong).length,backtracks:trace.filter(isBacktrack).length};$('subtitle').textContent=`${data.puzzleId} · ${data.solved?'Solved':'Unsolved'} · ${trace.length} recorded events`;[['Node metric',data.nodesExplored],['Solve time',Number(data.timeMs).toFixed(3)+' ms'],['Guesses',counts.guesses],['Wrong guesses',counts.wrong],['Backtracks',counts.backtracks]].forEach(([k,v])=>$('metrics').insertAdjacentHTML('beforeend',`<div class="metric"><b>${safe(v)}</b>${safe(k)}</div>`));
function boardAt(position){let board=fromText(data.startingBoard);for(let i=0;i<position;i++){const e=trace[i];if(isBacktrack(e)&&e.restoredBoard){board=fromText(e.restoredBoard);continue}if(e.placement&&(e.technique!=='Raw attempt'||e.valid))board[idx(e.placement.row,e.placement.col)]=e.placement.digit}return board}
function action(e){if(!e)return 'The original puzzle is ready.';if(isBacktrack(e))return `Return from r${e.placement.row+1}c${e.placement.col+1} = ${e.placement.digit}; restore the board before this branch.`;if(e.placement)return `${e.technique}: r${e.placement.row+1}c${e.placement.col+1} = ${e.placement.digit}`;const n=(e.eliminations||[]).length;return `${e.technique}: remove ${n} candidate${n===1?'':'s'}.`}
function outcome(e){if(!e)return 'Starting board';if(isBacktrack(e))return 'Backtrack: the failed branch has been fully undone.';if(e.valid===false)return 'Rejected legality check.';if(isWrong(e))return 'Wrong guess: this branch reaches a dead end.';if(e.isGuess)return 'Guess branch remains viable.';return 'Solver progress.'}
function drawBoard(){const board=boardAt(step),cur=step?trace[step-1]:null,b=$('board');b.replaceChildren();board.forEach((digit,i)=>{const cell=document.createElement('div'),given=data.startingBoard[i]!=='0'&&data.startingBoard[i]!=='.',current=cur?.placement&&idx(cur.placement.row,cur.placement.col)===i;cell.className='cell'+(current?' current':'')+(current&&isWrong(cur)?' wrong':'')+(current&&isBacktrack(cur)?' backtrack':'');if(digit){const text=document.createElement('span');text.className='digit '+(given?'given':'placed');text.textContent=digit;cell.append(text)}b.append(cell)})}
function renderEvent(){const cur=step?trace[step-1]:null;$('scrub').value=step;$('step-label').textContent=`Runtime event ${step} of ${trace.length}`;$('event-card').innerHTML=`<h3>${safe(cur?.technique||'Starting board')}</h3><p>${safe(action(cur))}</p><p class="status ${cur&&isWrong(cur)?'wrong-text':cur&&isBacktrack(cur)?'backtrack-text':''}">${safe(outcome(cur))}</p>`;document.querySelectorAll('.event-step').forEach((node,i)=>node.classList.toggle('selected',i===step-1))}
function render(){drawBoard();renderEvent()}function stop(){if(timer)clearInterval(timer);timer=null;$('play').textContent='Play'}function setStep(value){stop();step=Math.max(0,Math.min(trace.length,value));render()}$('scrub').max=trace.length;$('previous').onclick=()=>setStep(step-1);$('next').onclick=()=>setStep(step+1);$('reset').onclick=()=>setStep(0);$('scrub').oninput=e=>setStep(+e.target.value);$('play').onclick=()=>{if(timer){stop();return}if(step>=trace.length)step=0;$('play').textContent='Pause';timer=setInterval(()=>{step++;render();if(step>=trace.length)stop()},400)};document.addEventListener('keydown',e=>{if(e.code==='Space'){e.preventDefault();$('play').click()}else if(e.key==='ArrowRight')setStep(step+1);else if(e.key==='ArrowLeft')setStep(step-1);else if(e.key==='Home')setStep(0)});
const timeline=$('timeline');trace.forEach((e,i)=>{const b=document.createElement('button');b.type='button';b.className='event-step'+(isWrong(e)?' wrong':'')+(isBacktrack(e)?' backtrack':'');const label=isBacktrack(e)?`↩ r${e.placement.row+1}c${e.placement.col+1}`:e.placement?`r${e.placement.row+1}c${e.placement.col+1} = ${e.placement.digit}`:`${(e.eliminations||[]).length} eliminations`;b.innerHTML=`<span class="num">Step ${i+1}</span>${safe(e.technique)}<br>${safe(label)}`;b.onclick=()=>setStep(i+1);timeline.append(b)});render();})();
</script></body></html>)HTML";
    return static_cast<bool>(out);
}

bool TreeTraversalVisualizer::writeComparisonHtml(const std::array<SolveResult, 5>& results,
                                                   const std::array<std::string, 5>& solverNames,
                                                   const std::array<std::string, 5>& traceFilenames,
                                                   const std::string& puzzleId, const std::string& htmlPath)
{
    std::ofstream out(htmlPath);
    if (!out) return false;
    out << R"HTML(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Sudoku solver comparison</title><style>:root{color-scheme:dark;font-family:system-ui;background:#101821;color:#f2f7fa}body{margin:0}.page{max-width:1100px;margin:auto;padding:24px}h1{margin:0}.sub{color:#c0d0d8}table{width:100%;border-collapse:collapse;background:#172933;margin-top:18px}th,td{padding:10px;text-align:left;border-bottom:1px solid #415b6b}th{color:#ffb4bc}a{display:inline-block;padding:6px 9px;background:#d93645;color:#fff;text-decoration:none;border-radius:4px}.note{padding:12px;background:#25343d;border-left:4px solid #ef3e4d;margin-top:16px;color:#d4e0e6}</style></head><body><main class="page"><h1>Five-solver comparison</h1><p class="sub">)HTML" << escapeHtml(puzzleId) << R"HTML(</p><table><thead><tr><th>Solver</th><th>Time</th><th>Node metric</th><th>Guesses</th><th>Dead ends</th><th>Placements</th><th>Eliminations</th><th>Events</th><th>Trace</th></tr></thead><tbody>)HTML";
    out << std::fixed << std::setprecision(3);
    for (std::size_t index = 0; index < results.size(); ++index)
    {
        const TraceStats stats = summarize(results[index]);
        out << "<tr><td>" << escapeHtml(solverNames[index]) << "</td><td>" << results[index].timeMs << " ms</td><td>"
            << results[index].nodesExplored << (index < 2 ? " raw attempts" : " guesses") << "</td><td>"
            << stats.guesses << "</td><td>" << stats.failedGuesses << "</td><td>" << stats.placements << "</td><td>"
            << stats.eliminations << "</td><td>" << stats.events << "</td><td><a href=\"" << escapeHtml(traceFilenames[index]) << "\">Open trace</a></td></tr>";
    }
    out << R"HTML(</tbody></table><p class="note">Solver 1/2 count raw digit attempts; Solver 3–5 count branch guesses. Compare time, guesses, dead ends, and trace activity across all five solvers rather than treating node metrics as identical.</p></main></body></html>)HTML";
    return static_cast<bool>(out);
}
```

### src/treeTraversalVisualizer.hpp

```cpp
#pragma once

#include "solveResult.hpp"
#include <array>
#include <string>

// Writes one standalone HTML file. Its only input is the captured solve
// result, so the browser never needs the CSV dataset or a local web server.
class TreeTraversalVisualizer
{
public:
    static bool writeHtml(const SolveResult& result, const std::string& puzzleId,
                          const std::string& htmlPath);

    // Writes a chronological replay of every recorded solver event.  Unlike
    // writeHtml's solution-route player, this view includes wrong guesses,
    // dead ends, and the exact restored board after each backtrack.
    static bool writeRuntimeHtml(const SolveResult& result, const std::string& puzzleId,
                                 const std::string& htmlPath);

    // A research-facing overview that links the five individual trace pages.
    static bool writeComparisonHtml(const std::array<SolveResult, 5>& results,
                                    const std::array<std::string, 5>& solverNames,
                                    const std::array<std::string, 5>& traceFilenames,
                                    const std::string& puzzleId, const std::string& htmlPath);
};
```

### sudoku_solver.cbp

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
<CodeBlocks_project_file>
	<FileVersion major="1" minor="6" />
	<Project>
		<Option title="sudoku_solver" />
		<Option pch_mode="2" />
		<Option compiler="gcc" />
		<Build>
			<Target title="Debug">
				<Option output="bin/Debug/sudoku_solver" prefix_auto="1" extension_auto="1" />
				<Option object_output="obj/Debug/sudoku_solver/" />
				<Option type="1" />
				<Option compiler="gcc" />
				<Compiler>
					<Add option="-g" />
					<Add directory="src" />
				</Compiler>
			</Target>
			<Target title="Release">
				<Option output="bin/Release/sudoku_solver" prefix_auto="1" extension_auto="1" />
				<Option object_output="obj/Release/sudoku_solver/" />
				<Option type="1" />
				<Option compiler="gcc" />
				<Compiler>
					<Add option="-O3" />
					<Add option="-march=native" />
					<Add directory="src" />
				</Compiler>
			</Target>
			<Target title="Demo">
				<Option output="bin/Debug/sudoku_demo" prefix_auto="1" extension_auto="1" />
				<Option object_output="obj/Debug/sudoku_demo/" />
				<Option type="1" />
				<Option compiler="gcc" />
				<Compiler>
					<Add option="-g" />
					<Add directory="src" />
				</Compiler>
			</Target>
			<Target title="Master Collection">
				<Option output="bin/Release/master_collection_runner" prefix_auto="1" extension_auto="1" />
				<Option object_output="obj/Release/master_collection_runner/" />
				<Option type="1" />
				<Option compiler="gcc" />
				<Option working_dir="." />
				<Compiler>
					<Add option="-O3" />
					<Add option="-march=native" />
					<Add directory="src" />
				</Compiler>
			</Target>
			<Target title="Demo File">
				<Option output="bin/Debug/sudoku_demo_file" prefix_auto="1" extension_auto="1" />
				<Option object_output="obj/Debug/sudoku_demo_file/" />
				<Option type="1" />
				<Option compiler="gcc" />
				<Option working_dir="." />
				<Compiler>
					<Add option="-g" />
					<Add option="-O3" />
					<Add option="-march=native" />
					<Add directory="src" />
				</Compiler>
			</Target>
		</Build>
		<Compiler>
			<Add option="-Wall" />
			<Add option="-Wextra" />
			<Add option="-std=c++17" />
		</Compiler>
		<Linker>
			<Add option="-static" />
		</Linker>
		<Unit filename="src/Makefile" />
		<Unit filename="src/bitmaskBoard.hpp" />
		<Unit filename="src/bitmaskCandidateGrid.hpp" />
		<Unit filename="src/arrayCandidateGrid.hpp" />
		<Unit filename="src/board.hpp" />
		<Unit filename="src/candidateGrid.hpp" />
		<Unit filename="src/candidateSearch.hpp" />
		<Unit filename="src/data/sudoku_cluewise.csv" />
		<Unit filename="src/data/SUDOKU Master collection/17_Sudoku.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/17puz49157.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/17puz49158.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Almost Daily Nightmare_400.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/learningcurve_2500.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/No_One_Trick_Ponies_250.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Sudoku_diabolical.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Sudoku_easy.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Sudoku_hard.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Sudoku_medium.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/SUPER160.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Tarek_Pearly6000.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/data/SUDOKU Master collection/Top_50K_Toughest.txt">
			<Option compile="0" />
			<Option link="0" />
		</Unit>
		<Unit filename="src/datasetLoader.hpp" />
		<Unit filename="src/main.cpp">
			<Option target="Debug;Release" />
			<Option target="Debug" />
			<Option target="Release" />
		</Unit>
		<Unit filename="src/demo_main.cpp">
			<Option compilerVar="CPP" />
			<Option target="Demo" />
		</Unit>
		<Unit filename="src/demo_file_main.cpp">
			<Option compilerVar="CPP" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/master_collection_main.cpp">
			<Option compilerVar="CPP" />
			<Option target="Master Collection" />
		</Unit>
		<Unit filename="src/solveResult.hpp" />
		<Unit filename="src/trace/traceEvent.hpp" />
		<Unit filename="src/trace/jsonWriter.hpp" />
		<Unit filename="src/trace/jsonWriter.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
		</Unit>
		<Unit filename="src/solver1.hpp" />
		<Unit filename="src/solver2.hpp" />
		<Unit filename="src/solver3.hpp" />
		<Unit filename="src/arraySolver3.hpp" />
		<Unit filename="src/arraySolver3.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/solver3.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/solver4.hpp" />
		<Unit filename="src/arraySolver4.hpp" />
		<Unit filename="src/arraySolver4.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/solver4.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/solver5.hpp" />
		<Unit filename="src/arraySolver5.hpp" />
		<Unit filename="src/arraySolver5.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/solver5.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/cellSelection.hpp" />
		<Unit filename="src/new stuff/cellSelection.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/HeuristicEngine.hpp" />
		<Unit filename="src/new stuff/HeuristicEngine.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/hiddenSingle.hpp" />
		<Unit filename="src/new stuff/hiddenSingle.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/nakedPair.hpp" />
		<Unit filename="src/new stuff/nakedPair.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/nakedSingle.hpp" />
		<Unit filename="src/new stuff/nakedSingle.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/placement.hpp" />
		<Unit filename="src/new stuff/pointingPair.hpp" />
		<Unit filename="src/new stuff/pointingPair.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/new stuff/xWing.hpp" />
		<Unit filename="src/new stuff/xWing.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
			<Option target="Master Collection" />
			<Option target="Demo File" />
		</Unit>
		<Unit filename="src/treeLogger.hpp" />
		<Unit filename="src/treeTraversalVisualizer.hpp" />
		<Unit filename="src/treeTraversalVisualizer.cpp">
			<Option compilerVar="CPP" />
			<Option target="Debug;Release;Demo" />
			<Option target="Demo" />
			<Option target="Debug" />
			<Option target="Release" />
		</Unit>
		<Extensions>
			<lib_finder disable_auto="1" />
		</Extensions>
	</Project>
</CodeBlocks_project_file>
```

### tests/solve_statistics_test.cpp

```cpp
#include "solver1.hpp"
#include "solver2.hpp"
#include "solver3.hpp"
#include "solver4.hpp"
#include "solver5.hpp"
#include "arraySolver3.hpp"
#include "arraySolver4.hpp"
#include "arraySolver5.hpp"
#include "trace/jsonWriter.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
void require(bool condition, const std::string& message)
{
    if (!condition) throw std::runtime_error(message);
}

SolveResult solve(const Board& board, int solver, bool captureTrace
#ifndef STATISTICS_BASELINE
                  , SolveStatistics* statistics = nullptr
#endif
)
{
#ifdef STATISTICS_BASELINE
#define OPTIONS captureTrace
#else
#define OPTIONS captureTrace, statistics
#endif
    switch (solver)
    {
    case 1: return NaiveSolver::solve(board, OPTIONS);
    case 2: return BitmaskSolver::solve(board, OPTIONS);
    case 3: return Solver3::solve(board, OPTIONS);
    case 4: return Solver4::solve(board, OPTIONS);
    case 5: return Solver5::solve(board, OPTIONS);
    case 6: return ArraySolver3::solve(board, OPTIONS);
    case 7: return ArraySolver4::solve(board, OPTIONS);
    case 8: return ArraySolver5::solve(board, OPTIONS);
    case 9: return Solver3::solveWithFirstEmptyCell(board, OPTIONS);
    default: throw std::runtime_error("Invalid test solver");
    }
#undef OPTIONS
}

std::uint64_t traceDigest(SolveResult& result)
{
    // Ignore elapsed time; compare every serialized board, node and trace field.
    result.timeMs = 0.0;
    const std::string json = TraceJson::toJson(result, "statistics-test");
    std::uint64_t digest = 14695981039346656037ull;
    for (unsigned char character : json)
    {
        digest ^= character;
        digest *= 1099511628211ull;
    }
    return digest;
}

#ifndef STATISTICS_BASELINE
SolveStatistics summarizeTrace(const SolveResult& result)
{
    SolveStatistics expected;
    for (const TraceEvent& event : result.trace)
    {
        expected.maxDepth = std::max(expected.maxDepth, event.depth);
        expected.guesses += event.isGuess;
        expected.deadEnds += event.isGuess && event.leadsToConflict;
        expected.placements += event.placement.has_value();
        expected.eliminations += event.eliminations.size();
    }
    expected.events = result.trace.size();
    return expected;
}

void compareStatistics(const SolveStatistics& expected, const SolveStatistics& actual)
{
    require(expected.maxDepth == actual.maxDepth, "Maximum depth differs");
    require(expected.guesses == actual.guesses, "Guess count differs");
    require(expected.deadEnds == actual.deadEnds, "Dead-end count differs");
    require(expected.placements == actual.placements, "Placement count differs");
    require(expected.eliminations == actual.eliminations, "Elimination count differs");
    require(expected.events == actual.events, "Event count differs");
}

void compareResults(const SolveResult& expected, const SolveResult& actual)
{
    require(expected.solved == actual.solved, "Solved status differs");
    require(expected.board.toString() == actual.board.toString(), "Final board differs");
    require(expected.startingBoard.toString() == actual.startingBoard.toString(), "Starting board differs");
    require(expected.nodesExplored == actual.nodesExplored, "Search node count differs");
}
#endif
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc == 2, "Usage: solve_statistics_test EXAMPLES_FILE");
        std::ifstream input(argv[1]);
        require(static_cast<bool>(input), "Cannot open examples file");
        std::vector<std::string> puzzles;
        std::string line;
        while (std::getline(input, line) && puzzles.size() < 5)
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.size() == 81 && line.find_first_not_of("0123456789.") == std::string::npos)
                puzzles.push_back(line);
        }
        require(puzzles.size() == 5, "Expected five manageable source puzzles");
        const SolveResult solved = solve(Board::fromString(puzzles.front()), 5, false);
        require(solved.solved, "Fixture could not be solved");
        puzzles.push_back(solved.board.toString());
        std::string oneBlank = solved.board.toString();
        oneBlank[0] = '0';
        puzzles.push_back(oneBlank);
        puzzles.push_back("123456780000000009" + std::string(63, '0'));
#ifndef STATISTICS_BASELINE
        bool sawFailedBranch = false;
        bool sawEliminations = false;
        bool sawRawRejection = false;
#endif
        for (std::size_t fixture = 0; fixture < puzzles.size(); ++fixture)
        {
            const Board board = Board::fromString(puzzles[fixture]);
            for (int solver = 1; solver <= 9; ++solver)
            {
                SolveResult traced = solve(board, solver, true);
#ifndef STATISTICS_BASELINE
                const SolveStatistics expected = summarizeTrace(traced);
                // Reuse a nonzero collector to verify each solve resets all its fields.
                SolveStatistics statistics{123, 456, 789, 123, 456, 789};
                const SolveResult counted = solve(board, solver, false, &statistics);
                compareResults(traced, counted);
                compareStatistics(expected, statistics);
                require(counted.trace.empty() && counted.trace.capacity() == 0,
                        "Statistics-only solve retained trace storage");
                const SolveResult plain = solve(board, solver, false);
                compareResults(traced, plain);
                require(plain.trace.empty(), "Plain solve retained a trace");
                SolveResult both = solve(board, solver, true, &statistics);
                compareResults(traced, both);
                compareStatistics(expected, statistics);
                require(traceDigest(traced) == traceDigest(both), "Collector changed the full trace");
                sawFailedBranch = sawFailedBranch || expected.deadEnds > 0;
                sawEliminations = sawEliminations || expected.eliminations > 0;
                for (const TraceEvent& event : traced.trace)
                    sawRawRejection = sawRawRejection || (event.technique == TechniqueType::RawAttempt && !event.valid);
                if (fixture == puzzles.size() - 1) require(!traced.solved, "Expected unsolvable fixture");
#endif
                std::cout << fixture << ',' << solver << ',' << traceDigest(traced) << '\n';
            }
        }
#ifndef STATISTICS_BASELINE
        require(sawFailedBranch && sawEliminations && sawRawRejection, "Required event scenarios were not exercised");
        std::cerr << "PASS: 72 cases; statistics, trace-free solves, collector resets, boards, nodes, full traces.\n";
#endif
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

### tools/analyze_sudoku_benchmarks.py

```python
"""Reproduce the report from preserved source measurements; never rewrites inputs."""
from pathlib import Path
import sys, json, hashlib, subprocess, os, platform
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tmp/sudoku_analysis_deps'))
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy import stats

OUT=ROOT/'output/analysis'
OUT.mkdir(parents=True,exist_ok=True)
ASSETS=OUT/'figures'
ASSETS.mkdir(exist_ok=True)
NAMED=Path(r'C:\Users\malob\.codex\attachments\3cfd426d-cc26-43c5-aea5-cee45793a8e3\Pasted text.txt')
NOTEBOOK=Path(r'C:\Users\malob\.codex\attachments\dc0feb9f-84ae-4f7f-ba77-8d7bd03c948e\Pasted text.txt')
PRIMARY=ROOT/'results/master_collection_benchmark.csv'
HISTORY=ROOT/'results/master_collection_metrics.csv'
SOLVERS=['Naive','Bitmask','Solver3_bitmask','Solver3_array','Solver4_bitmask','Solver4_array','Solver5_bitmask','Solver5_array']
MAIN=['Naive','Bitmask','Solver3_bitmask','Solver4_bitmask','Solver5_bitmask']
SHORT={'Naive':'S1 naive','Bitmask':'S2 bitmask',**{f'Solver{i}_{r}':f'S{i} {r}' for i in [3,4,5] for r in ['array','bitmask']}}
COLORS=['#52606d','#3b82a0','#16836a','#55a98b','#bd742b','#db9d50','#7955a1','#a48bc0']
METRICS=['Time (ms)','Samples','Max depth','Guesses','Dead ends','Placements','Eliminations','Events']

def load(path):
    d=pd.read_csv(path,dtype={'Puzzle String':str})
    d['Solver']=d.Solver.str.replace(' ','_',regex=False)
    assert set(d.Solver)==set(SOLVERS)
    assert d['Puzzle String'].str.fullmatch('[0-9]{81}').all()
    assert not d.duplicated(['Puzzle','Solver']).any()
    assert (d.groupby('Puzzle').Solver.nunique()==8).all()
    for m in METRICS:d[m]=pd.to_numeric(d[m],errors='coerce')
    d['clues']=d['Puzzle String'].map(lambda s:sum(c!='0' for c in s))
    d['collection']=d.Puzzle.str.rsplit(':',n=1).str[0]
    return d

def pivot(d,m):
    return d.pivot_table(index='Puzzle String',columns='Solver',values=m,aggfunc='median').reindex(columns=SOLVERS)

def fmt(x,n=3):
    return 'n.a.' if pd.isna(x) else f'{x:,.{n}f}'

def table(headers,rows):
    return '\n'.join(['| '+' | '.join(headers)+' |','| '+' | '.join(['---']*len(headers))+' |']+['| '+' | '.join(map(str,r))+' |' for r in rows])+'\n'

def savefig(name):
    plt.savefig(ASSETS/(name+'.png'),dpi=190,bbox_inches='tight',facecolor='white')
    plt.savefig(ASSETS/(name+'.svg'),bbox_inches='tight',facecolor='white')
    plt.close()

def independent_valid(puzzle,solution):
    if len(solution)!=81 or not set(solution)<=set('123456789'):return False
    if any(a!='0' and a!=b for a,b in zip(puzzle,solution)):return False
    b=np.array(list(solution)).reshape(9,9)
    target=set('123456789')
    return all(set(b[i,:])==target and set(b[:,i])==target for i in range(9)) and all(set(b[r:r+3,c:c+3].flat)==target for r in [0,3,6] for c in [0,3,6])

d=load(PRIMARY); named=load(NAMED)
t=pivot(d,'Time (ms)').dropna(); nt=pivot(named,'Time (ms)').dropna()
assert (t>0).all().all() and (nt>0).all().all()
g=pivot(d,'Guesses'); ng=pivot(named,'Guesses')
gc=g.dropna(); common=gc.index.intersection(t.index)
gc=gc.loc[common]; tc=t.loc[common]
assert (gc.Naive==gc.Bitmask).all()
for level in [3,4,5]:
    assert (g[f'Solver{level}_array']==g[f'Solver{level}_bitmask']).all()
for metric in ['Guesses','Dead ends','Placements','Eliminations','Events']:
    assert (d.groupby(['Puzzle String','Solver'])[metric].nunique()<=1).all()
history=pd.read_csv(HISTORY,dtype={'puzzle':str,'final_board':str})
history['key']=history.collection+':'+history.source_line.astype(str)
j=d.merge(history,left_on=['Puzzle','Solver'],right_on=['key','solver'],validate='one_to_one')
assert len(j)==len(d) and (j['Puzzle String']==j.puzzle).all()
node_match=int((j.Guesses==j.nodes).sum())
assert node_match==int(j.Guesses.notna().sum())
stored_valid=sum(independent_valid(p,s) for p,s in zip(history.puzzle,history.final_board))
assert stored_valid==len(history)
assert (history.groupby(['collection','source_line']).final_board.nunique()==1).all()

# Freeze a small, inexpensive fixture set before fresh execution.
names=d.drop_duplicates('Puzzle String').set_index('Puzzle String').Puzzle
eligible=gc.index[(gc.Naive<15000)&(gc.Naive>0)]
q=gc.loc[eligible]
cases={}
cases['zero_guess']=q.index[(q.Solver3_bitmask==0)&(q.Naive>100)][0]
for category,a,b in [('naked_pair_help','Solver3_bitmask','Solver4_bitmask'),('xwing_help','Solver4_bitmask','Solver5_bitmask')]:
    ix=q.index[(q[b]<q[a])&(t.loc[q.index,b]<t.loc[q.index,a])]
    ratio=t.loc[ix,a]/t.loc[ix,b]
    cases[category]=ratio.idxmax()
ix=q.index[(q.Solver3_bitmask==q.Solver4_bitmask)&(q.Solver4_bitmask==q.Solver5_bitmask)&(q.Solver3_bitmask>0)&(t.loc[q.index,'Solver5_bitmask']>t.loc[q.index,'Solver3_bitmask'])]
cases['same_guesses_slower']=(t.loc[ix,'Solver5_bitmask']/t.loc[ix,'Solver3_bitmask']).idxmax()
fixtures={'known_fixture':'530070000600195000098000060800060003400803001700020006060000280000419005000080079',**cases}
for col,sub in d.groupby('collection'):
    ix=sub['Puzzle String'].drop_duplicates()
    small=[p for p in ix if p in gc.index and gc.at[p,'Naive']<2500]
    if small and small[0] not in fixtures.values():fixtures['collection_'+str(len(fixtures))]=small[0]
manifest=OUT/'sanity_manifest.txt'
manifest.write_text(''.join(k+'\t'+v+'\n' for k,v in fixtures.items()),encoding='utf-8')

compiler=Path(r'C:\Program Files\CodeBlocks\MinGW\bin\g++.exe')
exe=OUT/'sanity_check.exe'
sources=[ROOT/'tools/sudoku_benchmark_sanity.cpp']+[ROOT/f'src/{stem}.cpp' for stem in ['solver3','solver4','solver5','arraySolver3','arraySolver4','arraySolver5']]+sorted((ROOT/'src/new stuff').glob('*.cpp'))
env=os.environ.copy();env['PATH']=str(compiler.parent)+os.pathsep+env.get('PATH','')
if '--reuse-sanity' not in sys.argv:
    print('Building and running the current-code sanity check...',flush=True)
    build=subprocess.run([str(compiler),'-std=c++17','-O2','-static',*[str(p) for p in sources],'-o',str(exe)],capture_output=True,text=True,env=env,timeout=180)
    (OUT/'sanity_build_log.txt').write_text(build.stdout+build.stderr,encoding='utf-8')
    assert build.returncode==0,build.stderr
    run=subprocess.run([str(exe),str(manifest)],capture_output=True,text=True,env=env,timeout=120)
    (OUT/'sanity_results.tsv').write_text(run.stdout,encoding='utf-8')
    assert run.returncode==0,run.stderr
sanity=pd.read_csv(OUT/'sanity_results.tsv',sep='\t')
assert (sanity[['solved','valid','same_solution','trace_parity']]==1).all().all()
case_events={k:sanity[sanity['case']==k].set_index('solver') for k in cases}
assert case_events['naked_pair_help'].at['Solver4_bitmask','naked_pair_events']>0
assert case_events['xwing_help'].at['Solver5_bitmask','xwing_events']>0
fresh_agreement=[]
for label,p in fixtures.items():
    if label=='known_fixture':continue
    f=sanity[sanity['case']==label].set_index('solver')
    fresh_agreement.extend(f.at[s,'nodes']==g.at[p,s] for s in SOLVERS)
assert all(fresh_agreement)

plt.rcParams.update({'font.family':'DejaVu Sans','font.size':10,'axes.spines.top':False,'axes.spines.right':False,'axes.titleweight':'bold','figure.titlesize':15})
fig,axes=plt.subplots(1,2,figsize=(12,5),sharey=True)
for ax,stat in zip(axes,['mean','median']):
    vals=t.agg(stat).reindex(SOLVERS)
    ax.barh(range(8),vals,color=COLORS)
    ax.set_xscale('log');ax.set_yticks(range(8),[SHORT[s] for s in SOLVERS]);ax.invert_yaxis();ax.set_title(stat.capitalize()+' recorded time');ax.set_xlabel('Milliseconds (log scale)');ax.grid(axis='x',alpha=.18)
    for i,v in enumerate(vals):ax.text(v*1.07,i,fmt(v,4),va='center',fontsize=8)
    ax.set_xlim(vals.min()*.5,vals.max()*4)
fig.suptitle(f'Runtime across {len(t):,} distinct matched puzzles');fig.tight_layout();savefig('01_runtime')

clues=d.drop_duplicates('Puzzle String').set_index('Puzzle String').clues
clue_rows=[]
for clue,ix in clues.groupby(clues):
    ix=ix.index.intersection(t.index);tt=t.loc[ix]
    clue_rows.append([int(clue),len(ix),tt.Naive.median(),tt.Bitmask.median(),(tt.Naive/tt.Bitmask).median(),(tt.Bitmask/tt.Solver3_bitmask).median()])
clue_df=pd.DataFrame(clue_rows,columns=['clues','n','S1','S2','S1_over_S2','S2_over_S3'])
fig,axes=plt.subplots(1,2,figsize=(12,4.3))
for ax,c,title in zip(axes,['S1_over_S2','S2_over_S3'],['S1 / S2: bitmask checking','S2 / S3: propagation with MRV']):
    z=clue_df[clue_df.n>=10];ax.plot(z.clues,z[c],marker='o',color='#16836a');ax.axhline(1,color='#52606d',ls='--',lw=1);ax.set_xlabel('Initial clue count');ax.set_ylabel('Median paired speedup');ax.set_title(title);ax.grid(alpha=.18)
    if c=='S2_over_S3':ax.set_yscale('log')
fig.suptitle('Observed clue-count groups with at least 10 puzzles');fig.tight_layout();savefig('02_clue_counts')

transitions=[]
fig,axes=plt.subplots(1,2,figsize=(12,4.7))
for ax,a,b in zip(axes,['Solver3_bitmask','Solver4_bitmask'],['Solver4_bitmask','Solver5_bitmask']):
    change=g[b]-g[a];ratio=t[b]/t[a]
    ax.scatter(change,ratio,s=12,alpha=.45,color='#16836a');ax.axvline(0,color='#52606d',lw=1);ax.axhline(1,color='#52606d',ls='--',lw=1);ax.set_xscale('symlog',linthresh=1);ax.set_yscale('log');ax.set_xlabel('Change in guesses (new - previous)');ax.set_ylabel('New runtime / previous runtime');ax.set_title(SHORT[a]+' to '+SHORT[b]);ax.grid(alpha=.15)
    transitions.append({'a':a,'b':b,'n':len(change),'less':int((change<0).sum()),'same':int((change==0).sum()),'more':int((change>0).sum()),'faster':int((ratio<1).sum()),'less_faster':int(((change<0)&(ratio<1)).sum()),'less_slower':int(((change<0)&(ratio>1)).sum()),'same_slower':int(((change==0)&(ratio>1)).sum()),'median_time_ratio':float(ratio.median())})
fig.suptitle(f'Search savings and runtime changes on {len(t):,} complete heuristic traces');fig.tight_layout();savefig('03_tradeoffs')

correlations={}
fig,axes=plt.subplots(3,2,figsize=(11,11))
for ax,s in zip(axes.flat,MAIN):
    r=stats.spearmanr(gc[s],tc[s]);correlations[s]={'rho':float(r.statistic),'n':len(gc)}
    ax.scatter(gc[s]+1,tc[s],s=8,alpha=.3,color=COLORS[SOLVERS.index(s)]);ax.set_xscale('log');ax.set_yscale('log');ax.set_title(f'{SHORT[s]}: Spearman rho = {r.statistic:.3f}');ax.set_xlabel('Guesses + 1 (log scale)');ax.set_ylabel('Milliseconds (log scale)');ax.grid(alpha=.15)
axes.flat[-1].axis('off');fig.suptitle('More search generally costs time, but guesses do not measure all work');fig.tight_layout();savefig('04_guesses_runtime')

pairs=[('Naive','Bitmask'),('Bitmask','Solver3_bitmask'),('Solver3_bitmask','Solver4_bitmask'),('Solver4_bitmask','Solver5_bitmask')]+[(f'Solver{i}_array',f'Solver{i}_bitmask') for i in [3,4,5]]
rng=np.random.default_rng(20261009)
pair_stats=[]
for a,b in pairs:
    ratio=(t[a]/t[b]).to_numpy();boot=np.median(ratio[rng.integers(0,len(ratio),(2000,len(ratio)))],axis=1)
    logdiff=np.log(t[a])-np.log(t[b]);pval=stats.wilcoxon(logdiff).pvalue
    pair_stats.append({'a':a,'b':b,'n':len(ratio),'median_ratio':float(np.median(ratio)),'ci_low':float(np.quantile(boot,.025)),'ci_high':float(np.quantile(boot,.975)),'wins':int((ratio>1).sum()),'p':float(pval)})
ordered=sorted(range(len(pair_stats)),key=lambda i:pair_stats[i]['p']);running=0
for rank,i in enumerate(ordered):
    running=max(running,min(1,pair_stats[i]['p']*(len(ordered)-rank)));pair_stats[i]['p_holm']=running

quality=[]
for s in SOLVERS:
    z=d[d.Solver==s];quality.append([SHORT[s],len(z),int(z['Time (ms)'].notna().sum()),int(z.Events.isna().sum()),int((z.Samples==1).sum()),int(z.Samples.median())])
full_samples=pivot(d,'Samples');repeated=full_samples.index[(full_samples>=2).all(axis=1)]
sensitivity=[]
for label,ix in [('All matched timing puzzles',t.index),('Exclude any incomplete trace',common),('Require at least two samples for all solvers',repeated)]:
    z=t.loc[ix];sensitivity.append([label,len(z),z.Naive.mean(),z.Bitmask.mean(),(z.Naive/z.Bitmask).median(),(z.Bitmask/z.Solver3_bitmask).median()])
overlap=len(set(t.index)&set(nt.index))
provenance={str(p):{'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size} for p in [PRIMARY,NAMED,HISTORY,NOTEBOOK]}
summary={'title':'The Effects of Heuristics on Search Space and Work Done in Sudoku','primary':{'rows':len(d),'entries':d.Puzzle.nunique(),'distinct':len(t),'common_traces':len(common),'collections':d.collection.nunique()},'named':{'rows':len(named),'distinct':len(nt)},'overlap_between_cohorts':overlap,'quality':quality,'paired_comparisons':pair_stats,'transitions':transitions,'correlations':correlations,'sensitivity':sensitivity,'sanity':{'fixtures':len(fixtures),'solver_runs':len(sanity),'stored_valid':stored_valid,'node_matches':node_match,'fresh_historical_node_matches':len(fresh_agreement)},'cases':{k:{'puzzle':p,'id':names[p]} for k,p in cases.items()},'source_files':provenance}
(OUT/'analysis_summary.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
print(json.dumps(summary,indent=2))

# The written report is assembled from these calculated values below.
report=[]
def add(s=''):report.append(s+'\n')
add('# The Effects of Heuristics on Search Space and Work Done in Sudoku\n')
add('**Benchmark analysis | 9 October 2026**\n')
add('## Clear answer\n')
p23=pair_stats[1];tr34,tr45=transitions
add(f'Candidate propagation with MRV substantially reduces the explored search and recorded runtime on this benchmark. Across {len(t):,} distinct matched collection puzzles, Solver 3 with bitmask candidates has a median paired speedup of **{p23["median_ratio"]:.2f}x over basic bitmask backtracking**. Adding naked pairs or X-Wing reduces guesses on many puzzles, but usually increases runtime: Solver 4 is faster than Solver 3 on {int((t.Solver4_bitmask<t.Solver3_bitmask).sum()):,}/{len(t):,} puzzles, and Solver 5 is faster than Solver 4 on {int((t.Solver5_bitmask<t.Solver4_bitmask).sum()):,}/{len(t):,}. The useful conclusion is that additional deduction must earn back its processing cost on the actual puzzle distribution.\n')
add('These measurements describe explored branches and recorded work. They do not measure the complete mathematical solution space, CPU instructions, or energy use.\n')
add('## 1. Research frame\n')
add('**Title:** The Effects of Heuristics on Search Space and Work Done in Sudoku.\n')
add('1. **RQ1:** How does bitmask representation affect runtime when search policy is held fixed, and how does that effect vary with initial clue count?\n2. **RQ2:** How much do propagation and MRV change explored search and runtime relative to basic backtracking?\n3. **RQ3:** How do naked pairs and X-Wing change guesses and runtime on the same puzzles?\n4. **RQ4:** When do search savings translate into runtime savings, and when does added deduction work outweigh them?\n')
add('**Working hypothesis:** Propagation with MRV will reduce search substantially; representation changes will have a smaller and workload-dependent effect; richer deduction sets will help selected puzzles but may increase total runtime when their search savings are insufficient. This is a working interpretation developed while examining existing data, not a preregistered hypothesis tested on a new holdout sample.\n')
add('## 2. Data, correctness, and metric definitions\n')
add(table(['Experiment','Rows','Distinct puzzles','Role'],[['Collection benchmark',f'{len(d):,}',f'{len(t):,}','Main analysis'],['Supplied named-puzzle benchmark',len(named),len(nt),'Separate illustrative experiment']]))
add(f'The collection file contains {d.Puzzle.nunique():,} sampled collection entries from {d.collection.nunique()} collections. There are {d.Puzzle.nunique()-len(t)} repeated memberships beyond the distinct-puzzle count. The named-puzzle attachment and collection benchmark share {overlap} exact puzzle strings. They are not pooled, and timings from the two experiments must not be subtracted as if they came from the same build or measurement session.\n')
add('**Pairing and aggregation:** A row is one solver on one collection entry. Repeated memberships for the same exact puzzle and solver are collapsed to the median of their recorded timings. Overall results use the same distinct puzzles for all eight solvers. Search counters are likewise collapsed by median; their repeated values were checked for consistency. Collection summaries retain membership and should not be summed to reconstruct a distinct-puzzle total.\n')
add('**Timing:** The master runner normally exports the middle ordered trace-free timing from repeated solves, or a preserved calibration measurement if a later step fails. It measures around the solver call and collects trace metrics separately. The file does not contain the raw repeats, so the original mean runtime within each puzzle/run cannot be reconstructed. The mean below is a mean across per-puzzle recorded timings; it is not a mean of all raw timed repetitions.\n')
add('**Sampling:** The matching historical metrics file records seed **20260928**, requested **200 entries per collection**, a **15,000 ms** worker timeout, up to **31 timed runs**, and a **250 ms** timing target. The master source uses reservoir sampling with the seed combined with a filename hash. These are recovered settings from the matching historical records, not proof of a preserved invocation for the newer timing file. The sample was drawn by collection, not equally by clue count. Clue counts are derived from the 81-character strings and used for descriptive stratification.\n')
add('### Fresh sanity check\n')
add(f'A fresh GCC 14.2.0 C++17 build, using -O2 and static linking, checked **{len(fixtures)} fixtures across eight solvers ({len(sanity)} solver/fixture pairs)**. Every pair returned a valid completed grid, preserved givens, agreed on the solution, and produced matching traced/untraced results and node counts. The known fixture also matched its independently supplied expected grid. Each pair was executed once without tracing and once with tracing. The other {len(fixtures)-1} fixtures were selected from the benchmark, including the four case studies and inexpensive collection examples. All {len(fresh_agreement)} benchmark fixture/solver node counts matched their stored values. This is a current-code sanity check, not a fresh rerun of every historical timing or an exhaustive correctness proof.\n')
add(f'An independent audit also validated all **{stored_valid:,} stored final boards** in the older metrics file, including row/column/box constraints and original givens. All eight stored solutions agreed per entry. The older and newer files match on all {len(d):,} puzzle/solver keys and exact puzzle strings, and all {node_match:,} available newer guess counts equal the older node counts. This supports using the stored counters as explored-branch counts. It does not prove historical executable/source identity or retroactively certify the timing session.\n')
add('### What each counter means\n')
add(table(['Metric','Meaning and proper use'],[['Search nodes / Guesses','A legal guessed placement entering a recursive branch. In current code, all eight solvers use this definition; rejected digit tests and deterministic deductions are excluded.'],['Dead ends','Guess events marked as leading to conflict; failed guesses, not every trace event.'],['Max depth','Maximum recorded event depth. Baseline attempts start at depth 0 while heuristic guesses start at depth 1; compare within a family and avoid interpreting a one-level difference as an algorithmic benefit.'],['Placements','Events with a placement payload. Includes attempts and backtracks in the current implementation; not unique filled cells.'],['Eliminations','Explicit candidate removals attached to technique events; not all internal candidate updates.'],['Events','Trace records. One record may contain several changes; records differ between solver families. Not CPU operations.'],['Zero-guess solve','Guesses equals zero on a successful solve; success must be checked independently because the newer timing CSV has no solved field.']]))
add('The older project prose says that baseline nodes count all attempted digits. That is inconsistent with the current source and with the matching collection counters. Do not copy that statement into the new report. For the supplied 24-puzzle table, `Guesses` is treated as the labeled branch counter; its `Placements`/`Events` clearly follow a different logging convention from current baseline traces. Absolute event counts are therefore not pooled across experiments or presented as equal-cost units.\n')
add('### Missingness and repetition\n')
add(table(['Solver','Rows','Positive timing rows','Incomplete traces','Single-sample timings','Median samples'],quality))
add(f'All {len(d):,} collection timing rows are positive. Seven traces are incomplete, belonging to four puzzles. Overall timing comparisons retain their recorded timings; common-trace comparisons use **{len(common):,}** puzzles. The timing CSV lacks explicit solved/valid fields. Missing trace statistics are not replaced with zero, and a positive timing alone is not proof of success.\n')
add('## 3. RQ1: Representation and runtime\n')
add('### Main timing comparison\n')
timing_rows=[]
for s in SOLVERS:
    v=t[s];timing_rows.append([SHORT[s],fmt(v.mean(),6),fmt(v.median(),6),fmt(v.quantile(.95),6),fmt((t.Naive/v).median(),2)])
add(table(['Configuration','Mean ms','Median ms','P95 ms','Median paired S1/time'],timing_rows))
add('![Mean and median runtime](figures/01_runtime.png)\n')
add('Mean, median, and P95 answer different questions. The mean is sensitive to expensive tail puzzles; the median describes the middle puzzle; P95 marks the slowest 5% boundary. Paired speedup is the median of puzzle-by-puzzle ratios, not a ratio of aggregate medians.\n')
p12=pair_stats[0]
add(f'S1 and S2 have identical guess counts on all **{len(gc):,} common complete traces**. S2 is faster on **{p12["wins"]:,}/{len(t):,}** timing pairs. Its median paired speedup is **{p12["median_ratio"]:.3f}x**, with a paired puzzle-bootstrap interval of {p12["ci_low"]:.3f}-{p12["ci_high"]:.3f}x. That is a small typical representation benefit on this corpus. A speedup ratio of 1.026x means about 2.6% more throughput, or about 2.5% less elapsed time; these percentages are not interchangeable.\n')
add('The paired heuristic implementations produce identical guesses on all matched puzzles. Their timing comparison concerns the implemented candidate representation, initialization, updates, and state copying together. It does not isolate one bitwise instruction.\n')
add(table(['Stage','Median array/bitmask time','Bitmask faster / puzzles'],[[f'Solver {i}',fmt(pair_stats[i+1]["median_ratio"],3),f'{pair_stats[i+1]["wins"]:,}/{len(t):,}'] for i in [3,4,5]]))
add('### Average solving times by source collection\n')
collection_rows=[]
for collection,sub in d.groupby('collection'):
    group_times=pivot(sub,'Time (ms)').dropna()
    collection_rows.append([collection.replace('.txt',''),len(group_times),*[fmt(group_times[s].mean(),4) for s in MAIN]])
add(table(['Collection','Distinct entries','S1 mean ms','S2 mean ms','S3 mean ms','S4 mean ms','S5 mean ms'],collection_rows))
add('Each row uses matched puzzles within that collection; stages 3-5 use bitmask candidates. Collections overlap, so their row counts need not sum to the overall distinct count. These source labels are not calibrated difficulty levels.\n')
add('### Per-clue-count comparison\n')
add('![Clue-count comparisons](figures/02_clue_counts.png)\n')
add(table(['Clues','Distinct puzzles','S1 median ms','S2 median ms','Median S1/S2','Median S2/S3 bitmask'],[[r[0],r[1],fmt(r[2],5),fmt(r[3],5),fmt(r[4],3),fmt(r[5],2)] for r in clue_rows]))
add('The plot displays groups with at least ten puzzles; the table retains all observed groups. These are post-hoc groups from a collection sample, not balanced random samples at each clue count. Clue count does not hold puzzle structure or collection composition fixed, so a trend cannot establish that clue count causes runtime or difficulty.\n')
add('## 4. RQ2: Propagation with MRV and explored search\n')
node_rows=[]
for s in MAIN:
    v=gc[s];de=pivot(d,'Dead ends').loc[common,s]
    node_rows.append([SHORT[s],fmt(v.mean(),3),fmt(v.median(),1),fmt(de.mean(),3),int((v==0).sum())])
add(table(['Configuration','Mean guesses','Median guesses','Mean failed guesses','Zero-guess cases'],node_rows))
reduction=100*(1-gc.Solver3_bitmask/gc.Bitmask)
add(f'On the {len(common):,} common complete traces, Solver 3 reduces guesses relative to S2 by a **median {reduction.median():.4f}%** across paired puzzles. On all {len(t):,} heuristic trace records, the numbers of zero-guess cases are **{int((g.Solver3_bitmask==0).sum())} for S3**, **{int((g.Solver4_bitmask==0).sum())} for S4**, and **{int((g.Solver5_bitmask==0).sum())} for S5**. These totals use the full heuristic trace set; the table above uses the smaller common subset.\n')
add(f'Solver3_bitmask has a median paired speedup of **{p23["median_ratio"]:.2f}x over S2**. This is the combined effect of propagation, candidate-restricted branching, and MRV. The current benchmark does not isolate MRV, even though a first-empty-cell control exists in source. The measured node count describes the explored tree, not the number of all possible assignments.\n')
add('## 5. RQ3: Incremental deductions\n')
add(table(['Transition','Fewer guesses','Same guesses','More guesses','Faster runtime','Median new/old time'],[[SHORT[z['a']]+' to '+SHORT[z['b']],z['less'],z['same'],z['more'],f'{z["faster"]}/{z["n"]}',fmt(z['median_time_ratio'],3)] for z in transitions]))
add('![Search and runtime trade-offs](figures/03_tradeoffs.png)\n')
for z in transitions:
    add(f'For {SHORT[z["a"]]} to {SHORT[z["b"]]}, **{z["less_faster"]}** puzzles have fewer guesses and lower runtime, while **{z["less_slower"]}** have fewer guesses but higher runtime. Another **{z["same_slower"]}** have the same guess count and higher runtime. These comparisons use all {len(t):,} puzzles because the heuristic traces are complete.\n')
add('Additional deductions can change later MRV choices and the branch path. Soundness does not guarantee monotonic guess counts, and a guess-count increase is not itself an implementation defect. Equal guesses also do not prove an identical search tree. A slower solve with equal guesses is an observed cost without a guess-count benefit; calling it pure detector overhead would require more detailed instrumentation.\n')
add('## 6. RQ4: Search effort versus time\n')
add('### Recorded activity beyond guesses\n')
work_rows=[]
for s in SOLVERS:
    work_rows.append([SHORT[s],*[fmt(pivot(d,m).loc[common,s].mean(),2) for m in ['Max depth','Placements','Eliminations','Events']]])
add(table(['Configuration','Mean recorded max depth','Mean placement payloads','Mean explicit eliminations','Mean events'],work_rows))
add('These averages use the same 1,909 common complete traces. Depth origins and placement/event conventions differ between baselines and heuristic stages, as defined above. Compare these measures within a solver family; do not sum them into a total operation count or equate one baseline event with one heuristic event. The array/bitmask pairs have matching trace activity, helping separate search behavior from representation cost.\n')
add('![Guesses against runtime](figures/04_guesses_runtime.png)\n')
add(table(['Solver','Spearman rank correlation: guesses vs time','Puzzles'],[[SHORT[s],fmt(correlations[s]['rho'],5),len(common)] for s in MAIN]))
add('The correlations are calculated within each solver, using the same complete-trace puzzles. The plot uses guesses + 1 only to display zero-guess cases on a log axis; the correlation uses unshifted guesses. Positive correlation means harder search generally accompanies longer runtime. It does not estimate the time saved by removing a guess, and it does not establish that all work is proportional to guesses. Deduction scans, candidate updates, and state copying remain relevant.\n')
add('There is no defensible universal guess-count threshold where heuristics stop being worth it. The paired transition plot gives the answer puzzle by puzzle: a deduction is beneficial for runtime when its observed runtime ratio is below one. Explaining the mechanism requires the associated trace and, for exact costs, instrumentation of technique calls and time spent in each technique.\n')
add('## 7. Four case-study puzzles\n')
add('Cases below were selected from the collection benchmark to illustrate distinct behaviors, with a cap on baseline search cost so they could be freshly checked. The helpful cases require fewer guesses and lower stored runtime; the last case requires equal guesses and higher stored runtime. These are deliberately selected examples, not estimates of how common each behavior is. Fresh traces confirm that the named pair/X-Wing rules actually fire.\n')
for label,p in cases.items():
    add('### '+label.replace('_',' ').capitalize()+'\n')
    add(f'**Puzzle:** {names[p]}  \n**Initial clues:** {clues[p]}  \n**81-cell input:** `{p}`\n')
    e=case_events[label]
    add(table(['Stage','Guesses','Dead ends','Stored time ms','Fresh naked-pair events','Fresh X-Wing events'],[[SHORT[s],int(g.at[p,s]),int(pivot(d,'Dead ends').at[p,s]),fmt(t.at[p,s],6),int(e.at[s,'naked_pair_events']),int(e.at[s,'xwing_events'])] for s in ['Solver3_bitmask','Solver4_bitmask','Solver5_bitmask']]))
    if label=='zero_guess':add('Singles and pointing deductions finish this puzzle without branching in all three stages. Additional techniques have no guess-count saving to achieve here.\n')
    elif label=='naked_pair_help':add('Adding naked pairs produces fewer guesses and lower measured runtime. The fresh Solver 4 trace contains naked-pair events, supporting the technique-level explanation. Other later choices can also change, so this is the effect of enabling the stage as a whole.\n')
    elif label=='xwing_help':add('Adding X-Wing to Solver 4 produces fewer guesses and lower measured runtime. The fresh Solver 5 trace contains X-Wing events. This confirms actual use of the rule rather than inferring it only from a smaller guess count. Solver 5 is still slower than Solver 3 on this puzzle, illustrating that improving one stage does not establish the best configuration overall.\n')
    else:add('Solver 5 takes longer than Solver 3 with no reduction in guesses. This is a cost case for richer deduction on this input. It does not prove the traces are identical or that every additional microsecond comes from detector scanning.\n')
add('## 8. What the supplied 24-puzzle benchmark says\n')
add('This attachment is a separate historical experiment containing 24 named puzzles and 192 rows. Its timings use up to 100 samples, unlike the larger collection file. The older report describes these as selected behavioral examples; they are not a random sample of Sudoku puzzles.\n')
add(table(['Configuration','Mean ms','Median ms','Mean guesses'],[[SHORT[s],fmt(nt[s].mean(),4),fmt(nt[s].median(),4),fmt(ng[s].mean(),3)] for s in SOLVERS]))
named_ratio=nt.Naive/nt.Bitmask
ns=pivot(named,'Samples');nrepeat=ns.index[(ns[['Naive','Bitmask']]>=2).all(axis=1)]
anom=named[(named.Puzzle.str.contains('weak link',case=False)) & (named.Solver.isin(['Naive','Bitmask']))]
add(f'S2 is faster on **{int((named_ratio>1).sum())}/24** named puzzles, with a median paired speedup of **{named_ratio.median():.3f}x**. However, the mean S2 time is **{nt.Bitmask.mean():.3f} ms**, above S1 at **{nt.Naive.mean():.3f} ms**. The single-sample `GX- Cycle (weak link) x7` timings are **{nt.loc[anom["Puzzle String"].iloc[0],"Naive"]:.4f} ms for S1** and **{nt.loc[anom["Puzzle String"].iloc[0],"Bitmask"]:.4f} ms for S2**, reversing the pooled mean. This observation is retained. The attachment does not contain the remeasurement that would justify replacing it.\n')
add(f'If the baseline comparison is restricted to puzzles with at least two timing samples for both baselines, **{len(nrepeat)} puzzles** remain and the median S1/S2 ratio is **{(nt.loc[nrepeat,"Naive"]/nt.loc[nrepeat,"Bitmask"]).median():.3f}x**. This is a sensitivity result, not a correction to the raw table.\n')
add(table(['Transition','Fewer guesses','Same guesses','More guesses','Faster runtime / 24'],[[f'S{i} to S{i+1}',int((ng[f'Solver{i+1}_bitmask']<ng[f'Solver{i}_bitmask']).sum()),int((ng[f'Solver{i+1}_bitmask']==ng[f'Solver{i}_bitmask']).sum()),int((ng[f'Solver{i+1}_bitmask']>ng[f'Solver{i}_bitmask']).sum()),int((nt[f'Solver{i+1}_bitmask']<nt[f'Solver{i}_bitmask']).sum())] for i in [3,4]]))
add('The named set shows useful deductions without guaranteed runtime improvement, just as the collection set does. Its much larger typical array/bitmask timing differences do not transfer automatically to the collection experiment. They may reflect puzzle selection, build changes, machine state, or measurement differences. Those causes are not separately identifiable from the supplied summaries.\n')
add('## 9. Robustness and statistical support\n')
add(table(['Collection sensitivity subset','Puzzles','S1 mean ms','S2 mean ms','Median S1/S2','Median S2/S3'],[[r[0],r[1],fmt(r[2],4),fmt(r[3],4),fmt(r[4],3),fmt(r[5],2)] for r in sensitivity]))
add('The repeated-sample subset removes any puzzle with fewer than two samples for any solver. This is stronger than excluding incomplete traces. Both filters preferentially remove expensive baseline cases, so their estimates describe easier retained subsets and are not unbiased replacements for the full corpus.\n')
add(table(['Paired comparison A/B','Median A/B','95% puzzle-bootstrap interval','Holm-adjusted Wilcoxon p'],[[SHORT[z['a']]+' / '+SHORT[z['b']],fmt(z['median_ratio'],3),f'{z["ci_low"]:.3f}-{z["ci_high"]:.3f}',('<1e-300' if z['p_holm']<1e-300 else f'{z["p_holm"]:.3g}')] for z in pair_stats]))
add('Ratios above one favor B. Intervals use 2,000 paired puzzle bootstrap resamples with seed 20261009. Tests use paired log timings with Holm correction across the seven listed comparisons. Exact repeated puzzle strings are collapsed first. Related puzzle families may still weaken independence; signed-rank interpretation also depends on the paired log-difference distribution. These exploratory statistics describe this sample and do not repair historical timing noise. The intervals represent variation across sampled puzzles, not repeatability on new hardware or uncertainty from raw timing repetitions. The effect size and observed exceptions carry the practical conclusion.\n')
add('## 10. Discussion and limitations\n')
add('The most substantial improvement is associated with propagation plus MRV, which reduces explored search enough to offset its extra work on these inputs. Adding naked pairs and X-Wing continues to reduce typical guess counts and increases the number of deduction-only cases. However, the extra work often outweighs those search savings in elapsed time. Representation effects are smaller and vary across experiments. These findings support evaluating deduction sets against the expected input distribution rather than treating every stronger technique as an automatic speed improvement.\n')
add('- **Existing measurements:** The historical performance experiments were analysed, not rerun in full. Fresh executions establish a small correctness and counter-parity check.\n- **Success schema:** The newer CSV has no explicit solved/valid fields. Matching older stored boards are useful supporting evidence but cannot prove the newer session had identical outcomes.\n- **Sampling:** Ten collections and a separately selected named set are represented. This is not a balanced clue-count design or a random sample from all Sudoku puzzles.\n- **Timing:** Different repetition counts, calibration-only measurements, fixed solver order, missing raw timing repeats, and incomplete historical environment records limit precise attribution.\n- **Deduction attribution:** S2 to S3 combines propagation with MRV. S4/S5 isolate enabling additional configured rules, not the CPU time of each detector.\n- **Work metrics:** Event payloads and depth origins differ by solver family or logging version. Branch counts are useful, but event totals and time/event are not equal-cost operation counts.\n- **Selection and dependence:** Exact duplicates are removed for pooled comparisons. Related puzzle families can remain dependent, and case studies were selected after examining results.\n')
add('## 11. What to collect in the next benchmark\n')
add('Retain the current collection comparison as a pilot. Freeze the exact input manifest, seed, compiler/build identity, and machine details for the next run. If the question specifically concerns clue count, sample a defined number of puzzles within each clue stratum; report unavailable or small strata rather than calling the current collection sample balanced.\n')
add('Record one row per puzzle/configuration with explicit solved, valid, and outcome status; the branch-node count; failed guesses; normalized guess depth; zero-guess status; and actual timing sample count. Preserve all raw trace-free timings so both mean and median within a puzzle can be reported. Include the Solver 3 first-empty/MRV control if making a separate MRV claim. Counterbalance solver order and retain timeout records.\n')
add('For a direct work-done explanation, additionally record technique call counts, successful deductions, individual candidate removals, and time spent in each technique with instrumentation overhead assessed separately. This will allow detector overhead to be measured rather than inferred from equal guesses.\n')
add('## 12. Conclusion\n')
add(f'The collection benchmark supports a clear result: **propagation with MRV sharply reduces explored search and gives a typical {p23["median_ratio"]:.2f}x runtime improvement over basic bitmask backtracking, while richer deduction sets often trade fewer guesses for longer runtime.** Naked pairs and X-Wing have useful cases, but their value depends on the puzzle and the work needed to detect them. The appropriate claim concerns this implemented solver family and these benchmark inputs; it is not a universal ranking of Sudoku algorithms.\n')
add('## Evidence and reproducibility\n')
add('- Main measurements: [collection benchmark](../../results/master_collection_benchmark.csv).\n- Stored validity and matching branch counters: [historical metrics](../../results/master_collection_metrics.csv).\n- Separate experiment: the supplied 192-row named-puzzle attachment.\n- Supporting calculations and hashes: [analysis summary](analysis_summary.json).\n- Fresh executions: [sanity results](sanity_results.tsv) and [frozen fixture manifest](sanity_manifest.txt).\n- Reproduction: run `tools/analyze_sudoku_benchmarks.py` with the bundled Python environment and the dependencies described in its imports. It rebuilds the sanity harness, checks the stored data, and regenerates this report and figures.\n')
(OUT/'sudoku_heuristics_analysis.md').write_text('\n'.join(report).replace('\n\n\n','\n\n'),encoding='utf-8')
print('REPORT',OUT/'sudoku_heuristics_analysis.md')
```

### tools/run_snapshot_benchmark.ps1

```powershell
param(
    [string]$Compiler = 'C:/Program Files/CodeBlocks/MinGW/bin/g++.exe'
)
$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$destination = Join-Path $project 'output/snapshot_benchmark'
New-Item -ItemType Directory -Force $destination | Out-Null
$rows = Import-Csv (Join-Path $project 'results/master_collection_benchmark.csv')
$selected = @{}
# Select the ten most branching puzzles for EACH technique set; deduplicate
# by puzzle string. Include three zero-guess controls from Solver 3.
foreach ($solver in @('Solver3_bitmask', 'Solver4_bitmask', 'Solver5_bitmask')) {
    $heavy = $rows | Where-Object { $_.Solver -eq $solver } |
        Sort-Object { [long]$_.Guesses } -Descending | Select-Object -First 10
    foreach ($row in $heavy) { $selected[$row.'Puzzle String'] = $row.Puzzle }
}
$controls = $rows | Where-Object { $_.Solver -eq 'Solver3_bitmask' -and [long]$_.Guesses -eq 0 } |
    Select-Object -First 3
foreach ($row in $controls) { $selected[$row.'Puzzle String'] = $row.Puzzle }
$manifest = Join-Path $destination 'manifest.tsv'
$selected.GetEnumerator() | Sort-Object Value | ForEach-Object { "$($_.Value)`t$($_.Key)" } |
    Set-Content -Encoding ascii $manifest
$sources = @('tools/snapshot_cost_benchmark.cpp', 'src/solver3.cpp', 'src/solver4.cpp', 'src/solver5.cpp',
    'src/arraySolver3.cpp', 'src/arraySolver4.cpp', 'src/arraySolver5.cpp',
    'src/new stuff/HeuristicEngine.cpp', 'src/new stuff/cellSelection.cpp',
    'src/new stuff/nakedSingle.cpp', 'src/new stuff/hiddenSingle.cpp',
    'src/new stuff/nakedPair.cpp', 'src/new stuff/pointingPair.cpp', 'src/new stuff/xWing.cpp') |
    ForEach-Object { Join-Path $project $_ }
& $Compiler --version | Set-Content (Join-Path $destination 'compiler.txt')
foreach ($mode in @('baseline', 'profile')) {
    $executable = Join-Path $destination "$mode.exe"
    $flags = @('-std=c++17', '-O3', '-march=native', '-Wall', '-Wextra', '-static')
    if ($mode -eq 'profile') { $flags += '-DSUDOKU_PROFILE_SNAPSHOTS' }
    & $Compiler @flags @sources '-o' $executable
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $mode" }
    & $executable $manifest 1> (Join-Path $destination "$mode.csv") 2> (Join-Path $destination "$mode.log")
    if ($LASTEXITCODE -ne 0) { throw "Benchmark failed: $mode" }
}
foreach ($repeat in @('baseline_repeat', 'baseline_repeat2')) {
    & (Join-Path $destination 'baseline.exe') $manifest 1> (Join-Path $destination "$repeat.csv") 2> (Join-Path $destination "$repeat.log")
    if ($LASTEXITCODE -ne 0) { throw "Repeat benchmark failed: $repeat" }
}
$regression = Join-Path $destination 'statistics_test.exe'
$regressionSources = $sources | Select-Object -Skip 1
& $Compiler '-std=c++17' '-O3' '-march=native' '-Wall' '-Wextra' '-static' '-DSUDOKU_PROFILE_SNAPSHOTS' "-I$(Join-Path $project 'src')" (Join-Path $project 'tests/solve_statistics_test.cpp') (Join-Path $project 'src/trace/jsonWriter.cpp') @regressionSources '-o' $regression
if ($LASTEXITCODE -ne 0) { throw 'Regression build failed' }
& $regression (Join-Path $project 'src/data/sudokuwiki_examples.txt') 1> (Join-Path $destination 'statistics_test.csv') 2> (Join-Path $destination 'statistics_test.log')
if ($LASTEXITCODE -ne 0) { throw 'Regression failed' }
& (Join-Path $PSScriptRoot 'summarize_snapshot_benchmark.ps1')
Write-Output "Benchmark complete: $destination"
```

### tools/snapshot_cost_benchmark.cpp

```cpp
#include "../src/solver3.hpp"
#include "../src/solver4.hpp"
#include "../src/solver5.hpp"
#include "../src/arraySolver3.hpp"
#include "../src/arraySolver4.hpp"
#include "../src/arraySolver5.hpp"
#include "../src/new stuff/snapshotProfile.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using Solver = SolveResult (*)(const Board&, bool, SolveStatistics*);
static const Solver solvers[] = {Solver3::solve, ArraySolver3::solve,
    Solver4::solve, ArraySolver4::solve, Solver5::solve, ArraySolver5::solve};
static const char* names[] = {"Solver3_bitmask", "Solver3_array", "Solver4_bitmask",
    "Solver4_array", "Solver5_bitmask", "Solver5_array"};

void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

bool valid(const Board& board, const Board& given)
{
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c)
        {
            const int digit = board.cells[r][c];
            if (digit < 1 || digit > 9 || (given.cells[r][c] && given.cells[r][c] != digit)) return false;
            Board without = board;
            without.cells[r][c] = 0;
            if (!without.isValidPlacement(r, c, digit)) return false;
        }
    return true;
}

double median(std::vector<double> values)
{
    std::sort(values.begin(), values.end());
    return values.at(values.size() / 2);
}

// GCC memory barriers keep these real object copies alive under -O3.
// Repeated warm-cache copies are a separate estimate, not full-solve timing.
template<class Grid>
void micro(const char* name, const Board& original)
{
    Board board = original;
    Grid grid(board);
    std::vector<double> saves, restores;
    constexpr int repetitions = 1000000;
    for (int batch = 0; batch < 11; ++batch)
    {
        auto start = Clock::now();
        for (int i = 0; i < repetitions; ++i)
        {
            Board savedBoard = board;
            Grid savedGrid = grid;
            asm volatile("" : : "g"(&savedBoard), "g"(&savedGrid) : "memory");
        }
        saves.push_back(std::chrono::duration<double, std::nano>(Clock::now() - start).count() / repetitions);
        const Board savedBoard = board;
        const Grid savedGrid = grid;
        start = Clock::now();
        for (int i = 0; i < repetitions; ++i)
        {
            board = savedBoard;
            grid = savedGrid;
            asm volatile("" : : "g"(&board), "g"(&grid) : "memory");
        }
        restores.push_back(std::chrono::duration<double, std::nano>(Clock::now() - start).count() / repetitions);
    }
    std::cerr << "micro," << name << ",board_bytes," << sizeof(Board)
              << ",grid_bytes," << sizeof(Grid) << ",save_ns," << median(saves)
              << ",restore_ns," << median(restores) << '\n';
}

int main(int argc, char** argv)
{
    try
    {
        require(argc == 2, "Usage: snapshot_cost_benchmark manifest.tsv");
        std::ifstream input(argv[1]);
        require(bool(input), "Cannot read manifest");
        std::cerr << std::fixed << std::setprecision(3);
#ifdef SUDOKU_PROFILE_SNAPSHOTS
        std::vector<double> clockCosts;
        for (int i = 0; i < 100001; ++i)
        {
            const auto start = SnapshotProfile::Clock::now();
            clockCosts.push_back(SnapshotProfile::elapsed(start));
        }
        std::cerr << "empty_clock_pair_median_ns," << median(clockCosts) << '\n';
#endif
        std::cout << "puzzle,solver,median_ms,guesses,dead_ends,save_ns_raw,restore_ns_raw,solution\n";
        std::cout << std::fixed << std::setprecision(6);
        std::string line;
        bool first = true;
        while (std::getline(input, line))
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            const auto tab = line.find('\t');
            require(tab != std::string::npos, "Invalid manifest row");
            const std::string puzzle = line.substr(tab + 1);
            require(puzzle.size() == 81 && puzzle.find_first_not_of("0123456789.") == std::string::npos,
                    "Invalid puzzle");
            const Board board = Board::fromString(puzzle);
            if (first)
            {
                micro<BitmaskCandidateGrid>("bitmask", board);
                micro<ArrayCandidateGrid>("array", board);
                first = false;
            }
            SolveResult expected[6];
            SolveStatistics stats[6];
            for (int s = 0; s < 6; ++s)
            {
                expected[s] = solvers[s](board, false, &stats[s]);
                require(expected[s].solved && valid(expected[s].board, board), "Invalid solution");
                if (s % 2)
                {
                    require(expected[s].board.toString() == expected[s-1].board.toString(), "Representation changed solution");
                    require(expected[s].nodesExplored == expected[s-1].nodesExplored &&
                        stats[s].deadEnds == stats[s-1].deadEnds, "Representation changed search tree");
                }
                for (int warmup = 0; warmup < 3; ++warmup) solvers[s](board, false, nullptr);
            }
            std::vector<double> times[6], saves[6], restores[6];
            for (int sample = 0; sample < 31; ++sample)
                for (int step = 0; step < 6; ++step)
                {
                    // Rotate and reverse order to limit order/temperature bias.
                    const int s = sample % 2 ? (sample + 5 - step) % 6 : (sample + step) % 6;
#ifdef SUDOKU_PROFILE_SNAPSHOTS
                    SnapshotProfile::counters = {};
#endif
                    const auto result = solvers[s](board, false, nullptr);
                    require(result.solved && result.board.toString() == expected[s].board.toString() &&
                            result.nodesExplored == expected[s].nodesExplored, "Timed solve differs");
                    times[s].push_back(result.timeMs);
#ifdef SUDOKU_PROFILE_SNAPSHOTS
                    const auto& c = SnapshotProfile::counters;
                    require(c.saves == stats[s].guesses && c.restores == stats[s].deadEnds,
                            "Snapshot counters differ from search statistics");
                    saves[s].push_back(c.saveNs);
                    restores[s].push_back(c.restoreNs);
#else
                    saves[s].push_back(0);
                    restores[s].push_back(0);
#endif
                }
            for (int s = 0; s < 6; ++s)
                std::cout << line.substr(0, tab) << ',' << names[s] << ',' << median(times[s]) << ','
                          << stats[s].guesses << ',' << stats[s].deadEnds << ',' << median(saves[s]) << ','
                          << median(restores[s]) << ',' << expected[s].board.toString() << '\n';
            std::cerr << "checked," << line.substr(0, tab) << '\n';
        }
        require(!first, "Empty manifest");
        std::cerr << "PASS: solutions, givens, search parity, and snapshot counters (profile build).\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

### tools/sudoku_benchmark_sanity.cpp

```cpp
#include "../src/solver1.hpp"
#include "../src/solver2.hpp"
#include "../src/solver3.hpp"
#include "../src/solver4.hpp"
#include "../src/solver5.hpp"
#include "../src/arraySolver3.hpp"
#include "../src/arraySolver4.hpp"
#include "../src/arraySolver5.hpp"
#include <fstream>
#include <iostream>
#include <string>

bool valid(const Board& b, const Board& given) {
    for (int r=0;r<9;++r) {
        int rm=0,cm=0;
        for (int c=0;c<9;++c) {
            int v=b.cells[r][c],w=b.cells[c][r];
            if(v<1||v>9||w<1||w>9) return false;
            if(given.cells[r][c]&&given.cells[r][c]!=v) return false;
            rm|=1<<v;cm|=1<<w;
        }
        if(rm!=1022||cm!=1022) return false;
    }
    for(int box=0;box<9;++box) {
        int m=0;
        for(int i=0;i<9;++i) m|=1<<b.cells[box/3*3+i/3][box%3*3+i%3];
        if(m!=1022) return false;
    }
    return true;
}
SolveResult run(const Board& b,int s,bool trace) {
    switch(s) {
        case 0:return NaiveSolver::solve(b,trace);
        case 1:return BitmaskSolver::solve(b,trace);
        case 2:return Solver3::solve(b,trace);
        case 3:return Solver4::solve(b,trace);
        case 4:return Solver5::solve(b,trace);
        case 5:return ArraySolver3::solve(b,trace);
        case 6:return ArraySolver4::solve(b,trace);
        default:return ArraySolver5::solve(b,trace);
    }
}
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    std::ifstream in(argv[1]);
    if(!in) return 2;
    const char* names[]={"Naive","Bitmask","Solver3_bitmask","Solver4_bitmask","Solver5_bitmask","Solver3_array","Solver4_array","Solver5_array"};
    std::cout<<"case\tsolver\tsolved\tvalid\tsame_solution\ttrace_parity\tnodes\ttrace_guesses\tnaked_pair_events\txwing_events\ttrace_events\n";
    std::string line;
    int failures=0;
    while(std::getline(in,line)) {
        auto tab=line.find('\t');
        if(tab==std::string::npos) return 2;
        std::string label=line.substr(0,tab),p=line.substr(tab+1);
        if(p.size()!=81) return 2;
        Board b=Board::fromString(p);
        std::string canonical;
        for(int s=0;s<8;++s) {
            auto plain=run(b,s,false),trace=run(b,s,true);
            if(s==0) canonical=plain.board.toString();
            std::uint64_t guesses=0,np=0,xw=0;
            for(const auto& e:trace.trace) {
                guesses+=e.isGuess;
                np+=e.technique==TechniqueType::NakedPair;
                xw+=e.technique==TechniqueType::XWingRow||e.technique==TechniqueType::XWingCol;
            }
            bool v=plain.solved&&valid(plain.board,b);
            bool same=plain.board.toString()==canonical;
            bool parity=trace.solved==plain.solved&&trace.board.toString()==canonical&&trace.nodesExplored==plain.nodesExplored&&guesses==plain.nodesExplored;
            if(label=="known_fixture") same=same&&canonical=="534678912672195348198342567859761423426853791713924856961537284287419635345286179";
            failures+=!v||!same||!parity;
            std::cout<<label<<'\t'<<names[s]<<'\t'<<plain.solved<<'\t'<<v<<'\t'<<same<<'\t'<<parity<<'\t'<<plain.nodesExplored<<'\t'<<guesses<<'\t'<<np<<'\t'<<xw<<'\t'<<trace.trace.size()<<'\n';
        }
    }
    return failures?1:0;
}
```

### tools/summarize_snapshot_benchmark.ps1

````powershell
$ErrorActionPreference = 'Stop'
$destination = Join-Path (Split-Path $PSScriptRoot -Parent) 'output/snapshot_benchmark'
function Format-Number([double]$value, [string]$format = 'F3') {
    $value.ToString($format, [Globalization.CultureInfo]::InvariantCulture)
}
$baseline = @(Import-Csv (Join-Path $destination 'baseline.csv'))
$profile = @(Import-Csv (Join-Path $destination 'profile.csv'))
$identities = @($baseline | ForEach-Object { "$($_.puzzle),$($_.solver),$($_.guesses),$($_.dead_ends),$($_.solution)" })
$lines = [Collections.Generic.List[string]]::new()
$lines.Add('# Backtracking snapshot cost experiment')
$lines.Add('')
$lines.Add('Date: 12 October 2026. Compiler: GCC 14.2.0, C++17, -O3 -march=native -static, Windows x64. Traces and statistics collection are disabled during timed solves. Instrumentation is enabled only with SUDOKU_PROFILE_SNAPSHOTS; normal builds retain their original snapshot operations.')
$lines.Add('')
$lines.Add('## Finding')
$lines.Add('')
$lines.Add('The measurements do not support snapshot copying as the main explanation for an array advantage on heavy puzzles. Copy intervals, even including clock overhead, total less than 1% of baseline solving time in the profiled sample. Warm-cache copy estimates are smaller still. The historical array advantage was not consistently reproduced in the new full-solve runs. This does not establish which other operation explains historical timings.')
$lines.Add('')
$lines.Add('## Method and validation')
$lines.Add('')
$lines.Add('Select the ten highest-guess puzzles for each of bitmask Solvers 3, 4, and 5 from the existing master collection CSV, deduplicate puzzle strings, then add three zero-guess controls. This yields 13 branching puzzles and 3 controls. Run all six array/bitmask configurations on each puzzle with 3 warmups and 31 timed samples; rotate and reverse solver order. Report each puzzle/configuration median. Repeat the uninstrumented run three times. Selection uses old guess counts, not new timing results; this is a targeted sample, not a representative population estimate.')
$lines.Add('')
$lines.Add('Each solve must preserve givens and satisfy all Sudoku constraints. Array and bitmask versions must produce identical solutions, guesses and dead ends. Profile counters must equal guesses (saves) and dead ends (restores). Compare solution/count fields between all builds and runs. The existing 72-case regression suite passes with profiling enabled, including trace/statistics consistency and unsolvable cases.')
$lines.Add('')
$lines.Add('## Full solving times')
$lines.Add('')
$lines.Add('Times below are sums of per-puzzle medians over the 13 branching puzzles, not per-puzzle averages or median speedups. Array/bitmask below 1 means array is faster.')
$lines.Add('')
$lines.Add('| Run | Solver | Bitmask total ms | Array total ms | Array/bitmask |')
$lines.Add('|---|---|---:|---:|---:|')
foreach ($run in @('baseline', 'baseline_repeat', 'baseline_repeat2')) {
    $rows = @(Import-Csv (Join-Path $destination "$run.csv"))
    $other = @($rows | ForEach-Object { "$($_.puzzle),$($_.solver),$($_.guesses),$($_.dead_ends),$($_.solution)" })
    if (Compare-Object $identities $other) { throw "Results changed: $run" }
    foreach ($solver in 3..5) {
        $bm = ($rows | Where-Object { $_.solver -eq "Solver${solver}_bitmask" -and [long]$_.guesses -gt 0 } | Measure-Object median_ms -Sum).Sum
        $ar = ($rows | Where-Object { $_.solver -eq "Solver${solver}_array" -and [long]$_.guesses -gt 0 } | Measure-Object median_ms -Sum).Sum
        $lines.Add("| $run | $solver | $(Format-Number $bm) | $(Format-Number $ar) | $(Format-Number ($ar/$bm) 'F4') |")
    }
}
$other = @($profile | ForEach-Object { "$($_.puzzle),$($_.solver),$($_.guesses),$($_.dead_ends),$($_.solution)" })
if (Compare-Object $identities $other) { throw 'Profiling changed results' }
$lines.Add('')
$lines.Add('## In-solver copy timing')
$lines.Add('')
$lines.Add('Use a separate profiled build to time the actual save and restore statements. Values are raw observed intervals, include timer overhead, and are compared to uninstrumented baseline times. They are diagnostic measurements, not a precise counterfactual speedup from removing copies.')
$lines.Add('')
$lines.Add('| Solver | Saves | Restores | Raw copy ms | Raw copy / baseline time |')
$lines.Add('|---|---:|---:|---:|---:|')
foreach ($group in ($baseline | Where-Object { [long]$_.guesses -gt 0 } | Group-Object solver)) {
    $matches = @($profile | Where-Object { $_.solver -eq $group.Name -and [long]$_.guesses -gt 0 })
    $ms = ($group.Group | Measure-Object median_ms -Sum).Sum
    $ns = ($matches | ForEach-Object { [double]$_.save_ns_raw + [double]$_.restore_ns_raw } | Measure-Object -Sum).Sum
    $saves = ($group.Group | Measure-Object guesses -Sum).Sum
    $restores = ($group.Group | Measure-Object dead_ends -Sum).Sum
    $lines.Add("| $($group.Name) | $saves | $restores | $(Format-Number ($ns/1e6) 'F4') | $(Format-Number (100*$ns/1e6/$ms))% |")
}
$lines.Add('')
$lines.Add('Profiler timer calibration and independent warm-cache microbenchmarks (nanoseconds per board-plus-grid save or restore):')
$lines.Add('')
$lines.Add('```text')
foreach ($run in @('baseline', 'profile', 'baseline_repeat', 'baseline_repeat2')) {
    $lines.Add($run)
    foreach ($line in (Get-Content (Join-Path $destination "$run.log") | Where-Object { $_ -match '^(micro|empty_clock)' })) { $lines.Add($line) }
}
$lines.Add('```')
$lines.Add('')
$lines.Add('A Board occupies 324 bytes; BitmaskCandidateGrid occupies 552 bytes and includes a separate BitmaskBoard; ArrayCandidateGrid occupies 824 bytes. Object size alone does not predict optimized copy time. Microbenchmarks use compiler memory barriers to prevent dead-copy elimination and median batches of 1,000,000 copies. These copies are warm-cache and lack recursive stack/cache behaviour, so they are estimates rather than direct attribution. Individual clock intervals are quantized at about 100 ns on this host; per-copy timings are too small to interpret precisely with that clock. No timer-overhead subtraction is used to claim a precise copy cost.')
$lines.Add('')
$lines.Add('## Limits and conclusion')
$lines.Add('')
$lines.Add('Run-to-run total times vary, so a small representation advantage is not robust evidence of a particular mechanism. The controlled microbenchmark sometimes favours array copies, but its estimated saving is much smaller than millisecond-scale solving costs. We have measured the implemented mechanism; we have not performed a copy-free solver ablation. A causal claim would need a validated alternative restoration strategy and a larger, controlled comparison. The justified report wording is: "Snapshot copying occurs in both implementations, but targeted profiling found it contributed a small fraction of runtime and did not support it as the main cause of an array advantage."')
$lines.Add('')
$lines.Add('## Reproduce')
$lines.Add('')
$lines.Add('Run tools/run_snapshot_benchmark.ps1 (optionally supply -Compiler with a GCC path), then tools/summarize_snapshot_benchmark.ps1. Sources: tools/snapshot_cost_benchmark.cpp, src/new stuff/snapshotProfile.hpp, and the opt-in sections of src/new stuff/HeuristicEngine.cpp. Raw CSVs, manifests and logs are in this folder.')
$lines | Set-Content -Encoding utf8 (Join-Path $destination 'report.md')
Write-Output "Verified 96 solution/count pairs across all four runs. Report: $destination/report.md"
````

