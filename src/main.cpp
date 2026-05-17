#include "Game.hpp"

#include <iostream>
#include <string>

namespace {

struct Difficulty { int rows, cols, mines; };

void printUsage(const char* prog) {
    std::cerr
        << "Usage:\n"
        << "  " << prog << "                        # medium (default)\n"
        << "  " << prog << " easy | medium | hard\n"
        << "  " << prog << " <rows> <cols> <mines>\n"
        << "\n"
        << "Constraints: rows >= 3, cols >= 3, 0 < mines <= rows*cols - 9\n";
}

bool parseArgs(int argc, char** argv, Difficulty& out) {
    out = {16, 16, 40};  // medium default

    if (argc == 1) return true;

    if (argc == 2) {
        const std::string mode = argv[1];
        if (mode == "easy")   { out = {9,  9,  10}; return true; }
        if (mode == "medium") { out = {16, 16, 40}; return true; }
        if (mode == "hard")   { out = {16, 30, 99}; return true; }
        return false;
    }

    if (argc == 4) {
        try {
            out.rows  = std::stoi(argv[1]);
            out.cols  = std::stoi(argv[2]);
            out.mines = std::stoi(argv[3]);
        } catch (...) { return false; }

        // First-click safety relocates mines out of a 3x3 area, so we need
        // at least 9 non-mine tiles available.
        return out.rows >= 3
            && out.cols >= 3
            && out.mines > 0
            && out.mines <= out.rows * out.cols - 9;
    }

    return false;
}

}  // namespace

int main(int argc, char** argv) {
    Difficulty d;
    if (!parseArgs(argc, argv, d)) {
        printUsage(argv[0]);
        return 1;
    }

    Game game(d.rows, d.cols, d.mines);
    game.run();
    return 0;
}
