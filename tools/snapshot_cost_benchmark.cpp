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
