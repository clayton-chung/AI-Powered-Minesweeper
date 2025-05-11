#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <array>
#include <unordered_set>

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
    Board(int rows, int cols, float tileSize, int numMines);
    void draw(sf::RenderWindow& window);

    // Game logic
    void reset(int rows, int cols, int numMines);
    bool reveal(int x, int y);
    void flag(int x, int y);
    bool isCleared() const;
    bool chord (int x, int y);
    bool isRevealed(int x, int y) const;
    int  getAdjacentMines(int x, int y) const;
    int  flagCount() const;

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

    // Highlight state
    int highlightX_ = -1;
    int highlightY_ = -1;

    // Internal helpers
    void computeAdjacentMines();
};