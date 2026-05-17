# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

Requires SFML 2.5+ (graphics, window, system components).

```bash
mkdir build && cd build
cmake ..
make
./Minesweeper
```

The build copies `assets/` into the build directory automatically (via CMakeLists.txt). The executable must be run from the build directory so it can find `assets/mine-sweeper.ttf` at runtime.

## Testing

Catch2 v3 is pulled in via `FetchContent` and the test target is gated on `MINESWEEPER_BUILD_TESTS` (ON by default). Running `make` builds both `Minesweeper` and `test_board`. To run the suite:

```bash
cd build
ctest --output-on-failure        # or: ./tests/test_board
```

Tests live in `tests/test_board.cpp` and link directly against `src/board.cpp` (no separate library target). The test working directory is `build/tests/`, where the CMake script mirrors `assets/` so `Board`'s font load succeeds.

Board exposes a few APIs intended for tests and puzzle setup:
- `Board(rows, cols, tileSize, numMines, seed)` / `reset(..., seed)` — the optional `std::uint32_t` seed feeds the member `std::mt19937` for deterministic layouts.
- `placeMinesAt(indices, consumeFirstClick=true)` — bypasses random placement entirely; `consumeFirstClick=false` preserves first-click safety semantics so reveal() will still relocate mines.
- `mineCount()`, `hasMineAt(x, y)`, `isFlaggedAt(x, y)` — read-only inspection.

## Architecture

Three source files, two classes:

**`Board` (`include/Board.hpp`, `src/board.cpp`)** — owns the grid. Stores tiles as a flat `std::vector<Tile>` indexed by `y * cols_ + x`. Handles: mine placement, first-click safety (relocates mines out of a 3×3 safe zone on first reveal), flood-fill reveal, chord logic, flag toggling, and the AI solver. The AI solver (`AISolver()`) applies two constraint rules per revealed tile, performs one action per call, and sets `highlightX_/Y_` to the tile it acted on (used by `Game::render()` for the yellow highlight overlay).

**`Game` (`include/Game.hpp`, `src/game.cpp`)** — owns the SFML window, game loop, and UI. Follows a standard `processEvents()` / `update()` / `render()` loop. The AI runs on a clock-gated timer (`AISolveDelay_` = 200 ms) in `update()`. Window resizing is handled via a fixed `sf::View` (`gameView_`) that maps pixel coordinates back to world coordinates for hit-testing.

**`main.cpp`** — instantiates `Game` and calls `run()`.

## Difficulty Configuration

Difficulty is set at the CLI by `main.cpp`, which parses argv and passes `(rows, cols, numMines)` to the `Game` constructor:

```
./Minesweeper                       # medium (default)
./Minesweeper easy | medium | hard
./Minesweeper <rows> <cols> <mines>
```

Presets: Easy (9×9, 10), Medium (16×16, 40), Hard (16×30, 99). Custom mode requires `rows >= 3`, `cols >= 3`, and `mines <= rows*cols - 9` (the 3×3 first-click safe area must always fit).

## Controls

- **Left click** unrevealed tile: reveal. Left click a revealed numbered tile: chord (reveals all non-flagged neighbors if flagged-neighbor count matches the number).
- **Right click**: toggle flag.
- **Spacebar**: step the AI solver one move.
- **AI button** (bottom-right): toggle continuous AI solving at 200 ms/step.
- **Restart button** (smiley face, bottom-center): reset board.

## AI Solver Logic

`Board::AISolver()` scans all revealed tiles and applies two rules, returning after the first action taken:

1. If `flaggedNeighbors == adjacentMines` and there are unrevealed non-flagged neighbors → reveal them all.
2. If `unrevealedNeighbors == adjacentMines - flaggedNeighbors` (remaining mines == remaining unknowns) → flag them all.

Returns `false` when no deterministic move is available (an RNG guess is required).
