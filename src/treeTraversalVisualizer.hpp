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
