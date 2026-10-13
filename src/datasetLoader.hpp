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
