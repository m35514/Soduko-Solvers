# Sudoku solver

C++ Sudoku solver project with multiple solver implementations, deduction heuristics, benchmark tools, and HTML execution traces.

## Project layout

- `src/`: solvers, command-line programs, board representations, and tracing code.
- `src/new stuff/`: deduction heuristics and the heuristic engine.
- `src/data/`: included puzzle collections and examples.
- `tests/`: solver statistics checks.
- `tools/`: benchmark and analysis utilities.
- `sudoku_solver.cbp`: Code::Blocks project configuration.
- `PROJECT_CODE.md`: source-code reference document.

## Build

The Makefile requires a C++17 compiler (`g++`) and Make. From the project root:

```sh
cd src
make
```

On Windows, use a compatible Make and MinGW toolchain or open the Code::Blocks project with a configured C++ compiler.

Run the statistics checks with:

```sh
cd src
make test-statistics
```

## Local files excluded from Git

Compiled programs, IDE settings, temporary files, generated reports in `output/`, and benchmark results and execution traces in `results/` are kept locally.

The approximately 637 MiB dataset `src/data/sudoku_cluewise.csv` is also excluded because it exceeds GitHub's regular file-size limit. Workflows using that dataset require a separately obtained local copy; the smaller puzzle collections and examples are included in this repository.
