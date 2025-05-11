#include "Game.hpp"

Game::Game()
: window(sf::VideoMode(COLS * TILE_SIZE, ROWS * TILE_SIZE + 50), "AI-Powered Minesweeper")
, board(ROWS, COLS, TILE_SIZE, NUM_MINES)
{
    window.setFramerateLimit(FRAME_RATE);

    // Load font for numbers, flags, mine, etc.
    if (!font_.loadFromFile("assets/mine-sweeper.ttf")) {
        throw std::runtime_error("Font error");
    }

    // ========== Restart button ==========
    // --- Button positioning/dimensions ---
    float btnSize   = 40.f;
    float winW      = (float)window.getSize().x;
    float winH      = (float)window.getSize().y;
    float gridH     = ROWS * TILE_SIZE;
    float uiH       = winH - gridH;
    float btnX      = (winW - btnSize) / 2.f;
    float btnY      = gridH  + (uiH - btnSize) / 2.f;

    // --- Button appearance ---
    restartButton_.setSize({btnSize, btnSize});
    restartButton_.setFillColor({200,200,200});
    restartButton_.setPosition(btnX, btnY);

    // --- Face ---
    sf::Vector2f btnCenter = {
        btnX + btnSize/2.f,
        btnY + btnSize/2.f
    };

    float faceR = btnSize * 0.4f;
    smileFace_.setRadius(faceR);
    smileFace_.setFillColor(sf::Color::Yellow);
    smileFace_.setOutlineColor(sf::Color::Black);
    smileFace_.setOutlineThickness(2.f);
    smileFace_.setOrigin(faceR, faceR);
    smileFace_.setPosition(btnCenter);

    // --- Eyes ---
    float eyeR = faceR * 0.15f;
    eyeLeft_.setRadius(eyeR);
    eyeLeft_.setFillColor(sf::Color::Black);
    eyeLeft_.setOrigin(eyeR, eyeR);
    eyeLeft_.setPosition(btnCenter.x - faceR*0.4f, btnCenter.y - faceR*0.3f);

    eyeRight_ = eyeLeft_;
    eyeRight_.setPosition(btnCenter.x + faceR*0.4f, btnCenter.y - faceR*0.3f);

    // --- Smile ---
    smileMouth_.setSize({faceR * 0.6f, faceR * 0.1f});
    smileMouth_.setFillColor(sf::Color::Black);
    smileMouth_.setOrigin(smileMouth_.getSize().x/2.f, 0.f);
    smileMouth_.setPosition(btnCenter.x, btnCenter.y + faceR * 0.2f);

    // ========== Flag Counter ==========
    // --- Positioning ---
    float textY = gridH + (uiH - (float) flagIconText_.getCharacterSize()) / 2.f + 5.f;

    // --- Flag icon ---
    flagIconText_.setFont(font_);
    flagIconText_.setCharacterSize(18);
    flagIconText_.setString("`");
    flagIconText_.setFillColor(sf::Color(238,102,102));
    flagIconText_.setPosition(10.f, textY);

    // --- Flag count (number) ---
    float iconW = flagIconText_.getGlobalBounds().width + 5.f;
    flagCountText_.setFont(font_);
    flagCountText_.setCharacterSize(18);
    flagCountText_.setFillColor(sf::Color::White);
    flagCountText_.setPosition(10.f + iconW, textY);

    // ========== AI Button ==========
    AIButton_.setSize({100.f, 24.f});
    AIButton_.setFillColor({200,200,200});
    AIButton_.setPosition(winW - 10.f - 100.f, gridH + (uiH - 24.f)/2.f);

    AIButtonText_.setFont(font_);
    AIButtonText_.setCharacterSize(16);
    AIButtonText_.setString("AI: OFF");
    AIButtonText_.setFillColor(sf::Color::White);
    {
        auto b = AIButtonText_.getLocalBounds();
        AIButtonText_.setOrigin(b.width / 2.f + b.left, b.height / 2.f + b.top);
        AIButtonText_.setPosition(
            AIButton_.getPosition().x + AIButton_.getSize().x / 2.f,
            AIButton_.getPosition().y + AIButton_.getSize().y / 2.f
        );
    }

    // Set up the game view to allow resizing of window
    float worldW = COLS * TILE_SIZE;
    float worldH = ROWS * TILE_SIZE + uiH;
    gameView_.reset({0.f, 0.f, worldW, worldH});
    window.setView(gameView_);
}

// Main game loop
void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

// Handles user input and events
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed) {
            // Update the game view if the window is resized
            auto pixelPos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
            auto worldPos = window.mapPixelToCoords(pixelPos, gameView_);

            // Watch for restart button click
            sf::Vector2f mp(worldPos.x, worldPos.y);
            if (restartButton_.getGlobalBounds().contains(mp)) {
                board.reset(ROWS, COLS, NUM_MINES);
                state_ = GameState::PLAYING;
                continue;
            }

            // Ignore other clicks if game is not in PLAYING state
            if (state_ != GameState::PLAYING)
                continue;

            // Watch for AI button click
            if (AIButton_.getGlobalBounds().contains(mp)) {
                AISolveEnabled_ = !AISolveEnabled_;
                AIButtonText_.setString(AISolveEnabled_ ? "AI:   ON" : "AI: OFF");
                AIButton_.setFillColor(AISolveEnabled_ ? sf::Color(0, 255, 0) : sf::Color(200, 200, 200));
                AISolveClock_.restart();
                continue;
            }

            // Watch for tile click
            int x = worldPos.x / TILE_SIZE;
            int y = worldPos.y / TILE_SIZE;
            bool hit = false;
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (board.isRevealed(x,y) && board.getAdjacentMines(x,y) > 0) {
                    hit = board.chord(x, y);
                } else {
                    hit = board.reveal(x, y);
                }

                if (hit) {
                    state_ = GameState::LOSE;
                } else if (board.isCleared()) {
                    state_ = GameState::WIN;
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                board.flag(x, y);
            }
        }

        // Steps the AI solver when space is pressed
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
            if (state_ != GameState::PLAYING)
                continue;

            if (board.AISolver() && board.isCleared())
                state_ = GameState::WIN;
        }
    }
}

// Update game logic - AI on or off
void Game::update() {
    if (AISolveEnabled_ && state_ == GameState::PLAYING) {
        // Simulates "thinking" delay
        if (AISolveClock_.getElapsedTime() >=  AISolveDelay_){
            bool moved = board.AISolver();
            AISolveClock_.restart();

            // Checks win condition, turns AI off if game is won
            if (board.isCleared()) {
                state_ = GameState::WIN;
                AISolveEnabled_ = false;
                AIButtonText_.setString("AI: OFF");
                AIButton_.setFillColor(sf::Color(200, 200, 200));
            }
        }
    }
}

// Rendering/drawing the game
void Game::render() {
    window.setView(gameView_);
    window.clear();
    board.draw(window);

    // Highlights the tile the AI is currently working on
    int hx = board.getHighlightX();
    int hy = board.getHighlightY();
    if (hx >= 0 && hy >= 0) {
        sf::RectangleShape highlight;
        highlight.setSize({TILE_SIZE, TILE_SIZE});
        highlight.setFillColor(sf::Color(255, 255, 0, 50));
        highlight.setPosition(hx * TILE_SIZE, hy * TILE_SIZE);
        window.draw(highlight);
    }

    // Draw UI elements
    window.draw(restartButton_);
    window.draw(smileFace_);
    window.draw(eyeLeft_);
    window.draw(eyeRight_);
    window.draw(smileMouth_);
    int flagsLeft = NUM_MINES - board.flagCount();
    flagCountText_.setString(": " + std::to_string(flagsLeft));
    window.draw(flagIconText_);
    window.draw(flagCountText_);
    window.draw(AIButton_);
    window.draw(AIButtonText_);

    // Draw win/lose message
    if (state_ == GameState::WIN || state_ == GameState::LOSE) {
        sf::Text msg;
        msg.setFont(font_);
        msg.setCharacterSize(36);
        msg.setFillColor(sf::Color::White);
        msg.setOutlineColor(sf::Color::Black);
        msg.setOutlineThickness(4.f);
        msg.setString(state_ == GameState::WIN ? "You Win!" : "Game Over!");

        // Center the message in the window
        sf::View v = window.getView();
        sf::Vector2f center = v.getCenter();
        auto bounds = msg.getLocalBounds();
        msg.setOrigin(bounds.width / 2 + bounds.left, bounds.height / 2 + bounds.top);
        msg.setPosition(center.x, center.y - 50.f / 4);
        window.draw(msg);
        
        // Reset AI state if game is over
        AISolveEnabled_ = false;
        AIButtonText_.setString("AI: OFF");
        AIButton_.setFillColor(sf::Color(200, 200, 200));
    }

    window.display();
}