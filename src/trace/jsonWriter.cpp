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
