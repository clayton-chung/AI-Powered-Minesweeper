#include <catch2/catch_test_macros.hpp>

#include "Board.hpp"

#include <cstdint>
#include <vector>

namespace {

constexpr float         kTileSize = 50.f;
constexpr std::uint32_t kSeedA    = 0xC0FFEEu;
constexpr std::uint32_t kSeedB    = 0xDEADBEEFu;

std::vector<int> mineLayout(const Board& b, int rows, int cols) {
    std::vector<int> mines;
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (b.hasMineAt(x, y)) mines.push_back(y * cols + x);
        }
    }
    return mines;
}

}  // namespace

// =============================================================================
// Construction & determinism
// =============================================================================

TEST_CASE("Reset places exactly numMines mines", "[board][reset]") {
    Board b(16, 16, kTileSize, 40, kSeedA);
    REQUIRE(b.mineCount() == 40);

    b.reset(16, 16, 20);
    REQUIRE(b.mineCount() == 20);
}

TEST_CASE("Same seed produces identical mine layouts", "[board][determinism]") {
    Board a(16, 16, kTileSize, 40, kSeedA);
    Board b(16, 16, kTileSize, 40, kSeedA);
    REQUIRE(mineLayout(a, 16, 16) == mineLayout(b, 16, 16));
}

TEST_CASE("Different seeds produce different layouts", "[board][determinism]") {
    Board a(16, 16, kTileSize, 40, kSeedA);
    Board b(16, 16, kTileSize, 40, kSeedB);
    REQUIRE(mineLayout(a, 16, 16) != mineLayout(b, 16, 16));
}

TEST_CASE("Reset with a new seed produces a new layout", "[board][determinism]") {
    Board b(16, 16, kTileSize, 40, kSeedA);
    const auto before = mineLayout(b, 16, 16);

    b.reset(16, 16, 40, kSeedB);
    const auto after = mineLayout(b, 16, 16);

    REQUIRE(before != after);
}

// =============================================================================
// Basic API
// =============================================================================

TEST_CASE("inBounds correctly identifies in-range coordinates", "[board][bounds]") {
    Board b(9, 9, kTileSize, 10, kSeedA);

    REQUIRE(b.inBounds(0, 0));
    REQUIRE(b.inBounds(8, 8));
    REQUIRE_FALSE(b.inBounds(-1, 0));
    REQUIRE_FALSE(b.inBounds(0, -1));
    REQUIRE_FALSE(b.inBounds(9, 0));
    REQUIRE_FALSE(b.inBounds(0, 9));
}

TEST_CASE("reveal on out-of-bounds is a no-op", "[board][reveal]") {
    Board b(9, 9, kTileSize, 10, kSeedA);

    REQUIRE_FALSE(b.reveal(-1, 0));
    REQUIRE_FALSE(b.reveal(9, 0));
    REQUIRE_FALSE(b.reveal(0, 9));
    REQUIRE(b.flagCount() == 0);
}

TEST_CASE("flag toggles on unrevealed tiles", "[board][flag]") {
    Board b(9, 9, kTileSize, 10, kSeedA);

    b.flag(3, 3);
    REQUIRE(b.flagCount() == 1);
    REQUIRE(b.isFlaggedAt(3, 3));

    b.flag(3, 3);
    REQUIRE(b.flagCount() == 0);
    REQUIRE_FALSE(b.isFlaggedAt(3, 3));
}

TEST_CASE("flag is a no-op on revealed tiles", "[board][flag]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({});             // mine-free; first-click consumed
    b.reveal(2, 2);                 // 0-tile floods the whole board

    REQUIRE(b.isRevealed(2, 2));
    b.flag(2, 2);
    REQUIRE(b.flagCount() == 0);
}

// =============================================================================
// First-click safety
// =============================================================================

TEST_CASE("First click on a mine relocates it", "[board][first-click]") {
    Board b(9, 9, kTileSize, 0, kSeedA);
    b.placeMinesAt({4 * 9 + 4}, /*consumeFirstClick=*/false);
    REQUIRE(b.hasMineAt(4, 4));
    REQUIRE(b.mineCount() == 1);

    const bool hit = b.reveal(4, 4);

    REQUIRE_FALSE(hit);
    REQUIRE_FALSE(b.hasMineAt(4, 4));
    REQUIRE(b.mineCount() == 1);     // relocated, not lost
}

TEST_CASE("First click ensures a mine-free 3x3 safe area", "[board][first-click]") {
    Board b(9, 9, kTileSize, 0, kSeedA);

    std::vector<int> initialMines;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            initialMines.push_back((4 + dy) * 9 + (4 + dx));
        }
    }
    b.placeMinesAt(initialMines, /*consumeFirstClick=*/false);
    REQUIRE(b.mineCount() == 9);

    b.reveal(4, 4);

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            REQUIRE_FALSE(b.hasMineAt(4 + dx, 4 + dy));
        }
    }
    REQUIRE(b.mineCount() == 9);
}

TEST_CASE("First click only fires once; later reveals can hit mines", "[board][first-click]") {
    Board b(9, 9, kTileSize, 0, kSeedA);
    b.placeMinesAt({0}, /*consumeFirstClick=*/false);

    const bool first = b.reveal(8, 8);    // far from (0,0); consumes first-click
    REQUIRE_FALSE(first);

    const bool second = b.reveal(0, 0);   // still a mine; should now lose
    REQUIRE(second);
}

// =============================================================================
// Flood fill
// =============================================================================

TEST_CASE("Revealing a 0-tile flood-fills the connected region", "[board][flood]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({});  // mine-free; first-click consumed

    b.reveal(0, 0);

    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            REQUIRE(b.isRevealed(x, y));
        }
    }
}

TEST_CASE("Numbered tiles do not flood-fill", "[board][flood]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});                  // mine at (0,0)

    b.reveal(1, 0);                       // adjacentMines == 1, no flood
    REQUIRE(b.isRevealed(1, 0));
    REQUIRE_FALSE(b.isRevealed(2, 0));
    REQUIRE_FALSE(b.isRevealed(1, 1));
}

// =============================================================================
// Chord
// =============================================================================

TEST_CASE("Chord with matching flag count reveals safe neighbors", "[board][chord]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});

    b.reveal(1, 1);
    REQUIRE(b.getAdjacentMines(1, 1) == 1);

    b.flag(0, 0);

    const bool hit = b.chord(1, 1);
    REQUIRE_FALSE(hit);

    REQUIRE(b.isRevealed(0, 1));
    REQUIRE(b.isRevealed(1, 0));
    REQUIRE(b.isRevealed(2, 1));
    REQUIRE(b.isFlaggedAt(0, 0));
    REQUIRE_FALSE(b.isRevealed(0, 0));
}

TEST_CASE("Chord with mismatched flag count is a no-op", "[board][chord]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});

    b.reveal(1, 1);
    REQUIRE(b.flagCount() == 0);          // mismatch: adjacentMines is 1, flags is 0

    const bool hit = b.chord(1, 1);
    REQUIRE_FALSE(hit);
    REQUIRE_FALSE(b.isRevealed(0, 1));
    REQUIRE_FALSE(b.isRevealed(1, 0));
}

TEST_CASE("Chord hits a mine when the flag is mis-placed", "[board][chord]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});

    b.reveal(1, 1);
    b.flag(0, 1);                         // wrong tile, but satisfies flag count
    REQUIRE(b.flagCount() == 1);

    const bool hit = b.chord(1, 1);
    REQUIRE(hit);
}

// =============================================================================
// AI Solver
// =============================================================================

TEST_CASE("AISolver rule 1: all mines flagged -> reveal remaining", "[board][ai]") {
    Board b(5, 5, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});

    b.reveal(1, 1);
    b.flag(0, 0);

    const bool acted = b.AISolver();
    REQUIRE(acted);

    REQUIRE(b.isRevealed(0, 1));
    REQUIRE(b.isRevealed(1, 0));
    REQUIRE(b.isRevealed(2, 0));
    REQUIRE(b.isRevealed(0, 2));
    REQUIRE(b.isRevealed(2, 2));
}

TEST_CASE("AISolver rule 2: only unknowns are mines -> flag them", "[board][ai]") {
    Board b(3, 3, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});                  // mine at (0,0)

    b.reveal(1, 2);                       // 0-tile; flood-fills every non-mine
    REQUIRE_FALSE(b.isRevealed(0, 0));
    REQUIRE(b.isRevealed(1, 0));

    const bool acted = b.AISolver();
    REQUIRE(acted);
    REQUIRE(b.isFlaggedAt(0, 0));
}

TEST_CASE("AISolver returns false when no deterministic move exists", "[board][ai]") {
    Board b(3, 3, kTileSize, 0, kSeedA);
    b.placeMinesAt({0, 8});               // mines at (0,0) and (2,2)

    b.reveal(1, 1);                       // adjacentMines == 2, no flood
    REQUIRE(b.isRevealed(1, 1));

    const bool acted = b.AISolver();
    REQUIRE_FALSE(acted);
}

// =============================================================================
// Win condition
// =============================================================================

TEST_CASE("isCleared becomes true after all non-mines are revealed", "[board][win]") {
    Board b(3, 3, kTileSize, 0, kSeedA);
    b.placeMinesAt({0});

    REQUIRE_FALSE(b.isCleared());

    b.reveal(2, 2);                       // floods, revealing every non-mine tile
    REQUIRE(b.isCleared());
}
