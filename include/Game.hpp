#include "Board.hpp"

class Game {
public:
    // Public types & constants
    enum class GameState{PLAYING, WIN, LOSE};
    static constexpr int    FRAME_RATE = 60;

    // Construction & main loop
    Game(int rows, int cols, int numMines);
    void run();

private:
    // Configuration (set at construction; see main.cpp for difficulty presets)
    int                     rows_;
    int                     cols_;
    int                     numMines_;
    static constexpr float  TILE_SIZE       = 50.f;
    sf::Time                AISolveDelay_   = sf::milliseconds(200);

    // Core objects
    sf::RenderWindow    window;
    Board               board;
    sf::View            gameView_;
    sf::Clock           AISolveClock_;

    // Logical state
    GameState   state_              = GameState::PLAYING;
    bool        AISolveEnabled_     = false;

    // UI elements
    sf::Font            font_;
    sf::RectangleShape  restartButton_;
    sf::CircleShape     smileFace_;
    sf::CircleShape     eyeLeft_, eyeRight_;
    sf::RectangleShape  smileMouth_;
    sf::Text            flagIconText_;
    sf::Text            flagCountText_;
    sf::RectangleShape  AIButton_;
    sf::Text            AIButtonText_;

    // Helpers
    void processEvents();
    void update();
    void render();
};