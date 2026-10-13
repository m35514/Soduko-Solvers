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
