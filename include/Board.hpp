#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <array>
#include <unordered_set>
#include <optional>
#include <cstdint>

struct Tile {
    bool                revealed        = false;
    bool                flagged         = false;
    bool                mine            = false;
    int                 adjacentMines   = 0;
    sf::RectangleShape  shape;
};

class Board {
public:
    // Construction & drawing
    Board(int rows, int cols, float tileSize, int numMines,
          std::optional<std::uint32_t> seed = std::nullopt);
    void draw(sf::RenderWindow& window);

    // Game logic
    void reset(int rows, int cols, int numMines,
               std::optional<std::uint32_t> seed = std::nullopt);
    bool reveal(int x, int y);
    void flag(int x, int y);
    bool isCleared() const;
    bool chord (int x, int y);
    bool isRevealed(int x, int y) const;
    int  getAdjacentMines(int x, int y) const;
    int  flagCount() const;

    // Test / puzzle setup hooks
    // Replaces the current mine layout with mines at the given linear indices
    // (y * cols + x) and recomputes adjacency. By default consumes first-click
    // safety, treating the board as already in progress so reveal()'s next
    // call won't relocate the mines we just placed.
    void placeMinesAt(const std::vector<int>& indices, bool consumeFirstClick = true);
    int  mineCount()   const;
    bool hasMineAt   (int x, int y) const;
    bool isFlaggedAt (int x, int y) const;

    // Utilities
    int  index(int x, int y) const;
    bool inBounds(int x, int y) const;
    bool AISolver();
    int  getHighlightX() const;
    int  getHighlightY() const;

private:
    // Configuration
    int                 rows_, cols_;
    float               tileSize_;
    std::vector<Tile>   tiles;
    sf::Font            font_;
    bool                firstClick_ = true;
    std::mt19937        rng_;

    // Highlight state
    int highlightX_ = -1;
    int highlightY_ = -1;

    // Internal helpers
    void computeAdjacentMines();

    // Invokes fn(nx, ny, Tile&) for each in-bounds 8-neighbor of (x, y).
    template<typename F>
    void forEachNeighbor(int x, int y, F&& fn) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (inBounds(nx, ny)) fn(nx, ny, tiles[index(nx, ny)]);
            }
        }
    }
};
