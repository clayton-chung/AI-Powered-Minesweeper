#include "Board.hpp"

Board::Board(int rows, int cols, float tileSize, int numMines) 
: rows_(rows), cols_(cols), tileSize_(tileSize) {

    // Load font for numbers, flags, mine, etc.
    if (!font_.loadFromFile("assets/mine-sweeper.ttf")) {
        throw std::runtime_error("Font error");
    }

    tiles.resize(rows_*cols_);
    
    // Initialize tile shapes and positions
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            Tile& t = tiles[index(x,y)];

            t.shape.setSize({tileSize-1, tileSize-1});
            t.shape.setPosition(x * tileSize, y * tileSize);
        }
    }

    // Populate the board with mines and adjacent mine counts
    reset(rows_, cols_, numMines);
}

// Resets the board to a new, random state
void Board::reset(int rows, int cols, int numMines) {
    firstClick_ = true;
    rows_ = rows;
    cols_ = cols;
    tiles.resize(rows_ * cols_);

    // Clear all tiles
    for (auto& t : tiles) {
        t.revealed = false;
        t.flagged = false;
        t.mine = false;
        t.adjacentMines = 0;
    }

    // Randomly place mines
    std::vector<int> minePositions(rows_ * cols_);
    std::iota(minePositions.begin(), minePositions.end(), 0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(minePositions.begin(), minePositions.end(), gen);
    for (int i = 0; i < numMines; ++i) {
        tiles[minePositions[i]].mine = true;
    }

    // Compute number of adjacent mines for each tile
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            Tile& t = tiles[index(x,y)];
            if (t.mine) continue;

            int adjacentCount = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx==0 && dy==0) continue;

                    int newX = x + dx;
                    int newY = y + dy;
                    if (inBounds(newX, newY) && tiles[index(newX, newY)].mine) {
                        ++adjacentCount;
                    }
                }
                
                t.adjacentMines = adjacentCount;
            }
        }
    }

    // Reset highlight state
    highlightX_ = -1;
    highlightY_ = -1;
}

void Board::draw(sf::RenderWindow& window) {
    // Lookup table for number colors.
    static const std::array<sf::Color,9> numberColors = {
        sf::Color::Transparent,     // 0, unused
        {124,199,255},              // 1
        {99,193,99},                // 2
        {255,119,136},              // 3
        {238,136,255},              // 4
        {221,170,34},               // 5
        {0,128,128},                // 6
        {0,0,0},                    // 7
        {128,128,128}               // 8 
    };

    sf::Text text;
    text.setFont(font_);
    text.setCharacterSize(tileSize_ / 2);

    for (auto& t : tiles) {
        if (!t.revealed) {      // Unrevealed tile
            t.shape.setFillColor(sf::Color(76,84,92));
        } else if (t.mine) {    // Revealed mine
            t.shape.setFillColor(sf::Color(238,102,102));
        } else {                // Revealed non-mine tile
            t.shape.setFillColor(sf::Color(51,58,65));
        }
        window.draw(t.shape);

        sf::Vector2f tilePos = t.shape.getPosition();
        float cx = tilePos.x + tileSize_ / 2;
        float cy = tilePos.y + tileSize_ / 2;

        // Flagged tile
        if (!t.revealed && t.flagged) {
                text.setString("`");
                auto bounds = text.getLocalBounds();
                text.setOrigin(bounds.width / 2 + bounds.left, bounds.height / 2 + bounds.top);
                text.setPosition(cx, cy);
                text.setFillColor(sf::Color(238,102,102));
                window.draw(text);
        }

        if (t.revealed) {
            // Mine tile
            if (t.mine) {
                text.setString("*");
                auto bounds = text.getLocalBounds();
                text.setOrigin(bounds.width / 2 + bounds.left, bounds.height / 2 + bounds.top);
                text.setPosition(cx, cy);
                text.setFillColor(sf::Color::Black);
                window.draw(text);
            // Number tile
            } else if (t.adjacentMines > 0) {
                text.setString(std::to_string(t.adjacentMines));
                auto bounds = text.getLocalBounds();
                text.setOrigin(bounds.width / 2 + bounds.left, bounds.height / 2 + bounds.top);
                text.setPosition(cx, cy);
                text.setFillColor(numberColors[t.adjacentMines]);
                window.draw(text);
            }
        }
    }
}


bool Board::reveal(int x, int y) {
    if (!inBounds(x,y)) return false;

    // "First click safe" rule: guarantee atleast 3x3 area around the first click is safe
    if (firstClick_) {
        firstClick_ = false;

        // Build a set of safe tiles around the first click
        std::unordered_set<int> safeTiles;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int newX = x + dx;
                int newY = y + dy;
                if (inBounds(newX, newY)) {
                    safeTiles.insert(index(newX, newY));
                }
            }
        }

        // Gather the mines in the safe area
        std::vector<int> toMove;
        for (int safeTile : safeTiles) {
            if (tiles[safeTile].mine) {
                toMove.push_back(safeTile);
                tiles[safeTile].mine = false;
            }
        }

        // Find new tiles for mines in the safe area
        std::vector<int> pool;
        pool.reserve(rows_ * cols_);
        for (int i = 0, N = rows_*cols_; i < N; ++i) {
            if (!tiles[i].mine && safeTiles.count(i) == 0) {
                pool.push_back(i);
            }
        }
        std::shuffle(pool.begin(), pool.end(), std::mt19937(std::random_device()()));

        for (int i = 0; i < (int)toMove.size(); ++i) {
            tiles[pool[i]].mine = true;
        }

        // Recompute adjacent mines after moving mines that were in the safe area
        computeAdjacentMines();
    }

    Tile& t = tiles[index(x,y)];
    
    if (t.revealed || t.flagged) return false;

    t.revealed = true;
    if (t.mine) {
        return true;
    }
    
    // Flood fill/reveal adjacent tiles if this tile has no adjacent mines.
    if (!t.mine && t.adjacentMines == 0) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx==0 && dy==0) continue;

                int newX = x + dx;
                int newY = y + dy;
                reveal(newX, newY);
            }
        }
    }

    return false;
}

// Toggles flag on a tile
void Board::flag(int x, int y) {
    if (!inBounds(x,y)) return;

    Tile& t = tiles[index(x,y)];

    // Toggle flag only if the tile is not revealed.
    if (!t.revealed)
        t.flagged = !t.flagged;
}

// Win condition: all non-mine tiles are revealed.
bool Board::isCleared() const {
    for (auto& t : tiles) {
        if (!t.revealed && !t.mine) {
            return false;
        }
    }
    return true;
}

// Chord: clicking on a revealed tile reveals all adjacent unrevealed tiles
//        that are not flagged. Only allowed if
//        number of adjacent flagged tiles == number of adjacent mines
bool Board::chord(int x, int y) {
    if (!inBounds(x,y)) return false;

    Tile& t = tiles[index(x,y)];

    if (!t.revealed || t.flagged) return false;

    // Count number of flagged neighbors
    int flags = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx==0 && dy==0) continue;

            int newX = x + dx;
            int newY = y + dy;
            if (inBounds(newX, newY) && tiles[index(newX, newY)].flagged) {
                ++flags;
            }
        }
    }

    if (flags != t.adjacentMines) {
        return false;
    }

    // Number of adjacent flagged tiles == number of adjacent mines, proceed.
    bool hit = false;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx==0 && dy==0) {
                continue;
            }

            int newX = x+dx;
            int newY = y+dy;
            if (!inBounds(newX, newY)) {
                continue;
            }

            Tile& n = tiles[index(newX, newY)];

            // Skip flagged tiles
            if (n.flagged) {
                continue;
            }

            // Unrevealed tile is a mine
            if (n.mine) {
                n.revealed = true;
                hit = true;
            } else {
                reveal(newX, newY);
            }
        }
    }

    return hit;
}

// Returns true if the tile at (x,y) is revealed
bool Board::isRevealed(int x, int y) const {
    return inBounds(x,y) && tiles[index(x,y)].revealed;
}

// Returns the number of adjacent mines for the tile at (x,y)
int Board::getAdjacentMines(int x, int y) const {
    return inBounds(x,y) ? tiles[index(x,y)].adjacentMines : 0;
}

// Returns the number of flagged tiles
int Board::flagCount() const {
    int count = 0;
    for (auto& t : tiles) {
        if (t.flagged) {
            ++count;
        }
    }
    return count;
}

// Computes the number of adjacent mines for each tile
void Board::computeAdjacentMines() {
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            Tile& t = tiles[index(x,y)];
            t.adjacentMines = 0;

            if (t.mine) continue;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx==0 && dy==0) continue;

                    int newX = x + dx;
                    int newY = y + dy;
                    if (inBounds(newX, newY) && tiles[index(newX, newY)].mine) {
                        ++t.adjacentMines;
                    }
                }
            }
        }
    }
}

// Attempts to solve the board using 2 simple rules:
//      1)  For any revealed tile where
//          flaggedNeighbours == adjacentMines,
//          reveal all adjacent unrevealed tiles.
//      2)  For any revealed tile where
//          unrevealedNeighbours == adjacentMines - flaggedNeighbours,
//          flag all adjacent unrevealed tiles.
bool Board::AISolver() {
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            Tile& t = tiles[index(x,y)];
            if (!t.revealed || t.mine) continue;

            highlightX_ = x;
            highlightY_ = y;

            // Counts flagged and unrevealed neighbors.
            int flagCount = 0;
            int unrevealedCount = 0;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx==0 && dy==0) continue;

                    int newX = x + dx;
                    int newY = y + dy;
                    if (!inBounds(newX, newY)) continue;
                    Tile& n = tiles[index(newX, newY)];

                    if (n.flagged) {
                        ++flagCount;
                    } else if (!n.revealed) {
                        ++unrevealedCount;
                    }
                }
            }

            // Rule 1
            if (flagCount == t.adjacentMines && unrevealedCount > 0) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx==0 && dy==0) continue;

                        int newX = x + dx;
                        int newY = y + dy;
                        if (!inBounds(newX, newY)) continue;
                        Tile& n = tiles[index(newX, newY)];

                        if (!n.revealed && !n.flagged) {
                            reveal(newX, newY);
                        }
                    }
                }
                return true;
            }

            // Rule 2
            int minesLeft = t.adjacentMines - flagCount;
            if (minesLeft > 0 && unrevealedCount == minesLeft) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx==0 && dy==0) continue;

                        int newX = x + dx;
                        int newY = y + dy;
                        if (!inBounds(newX, newY)) continue;
                        Tile& n = tiles[index(newX, newY)];

                        if (!n.revealed && !n.flagged) {
                            n.flagged = true;
                        }
                    }
                }
                return true;
            }
        }
    }

    // Reset highlight state if no moves were made
    highlightX_ = -1;
    highlightY_ = -1;
    return false;
}

// Returns the index of the tile at (x,y)
int Board::index(int x, int y) const {
    return y * cols_ + x;
}

// Returns true if the coordinates (x,y) are within the bounds of the board
bool Board::inBounds(int x, int y) const {
    return x >= 0 && x < cols_ && y >= 0 && y < rows_;
}

// Returns the x-coordinate of the highlighted tile
int Board::getHighlightX() const {
    return highlightX_;
}

// Returns the y-coordinate of the highlighted tile
int Board::getHighlightY() const {
    return highlightY_;
}