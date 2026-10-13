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
