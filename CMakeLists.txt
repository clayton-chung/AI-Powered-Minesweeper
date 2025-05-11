cmake_minimum_required(VERSION 3.10)
project(Minesweeper)

set(CMAKE_CXX_STANDARD 17)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_BINARY_DIR})

add_executable(Minesweeper
    src/main.cpp
    src/game.cpp
    src/board.cpp
)

target_include_directories(Minesweeper PRIVATE include)
target_link_libraries(Minesweeper sfml-graphics sfml-window sfml-system)
