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
