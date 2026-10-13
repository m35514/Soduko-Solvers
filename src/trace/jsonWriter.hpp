#pragma once

#include "../solveResult.hpp"
#include <string>

namespace TraceJson
{
    // Produces the complete, self-contained data payload embedded by the
    // visualizer.  No external JSON dependency is needed for this numeric data.
    std::string toJson(const SolveResult& result, const std::string& puzzleId);
}
