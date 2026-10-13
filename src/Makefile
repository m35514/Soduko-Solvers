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
