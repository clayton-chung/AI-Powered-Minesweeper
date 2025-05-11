#include "Board.hpp"

class Game {
public:
    // Public types & constants
    enum class GameState{PLAYING, WIN, LOSE};
    static constexpr int    FRAME_RATE = 60;

    // Construction & main loop
    Game();
    void run();

private:
    // Configuration
    // Difficulty may be adjusted here. Suggested difficulties:
    //      Easy:   9  rows x 9  cols, 10 mines
    //      Medium: 16 rows x 16 cols, 40 mines
    //      Hard:   16 rows x 30 cols, 99 mines
    static constexpr int    ROWS            = 16;
    static constexpr int    COLS            = 16;
    static constexpr int    NUM_MINES       = 40;
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