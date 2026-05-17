cmake_minimum_required(VERSION 3.14)
project(Minesweeper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

option(MINESWEEPER_BUILD_TESTS "Build the Catch2 unit test suite" ON)
if(MINESWEEPER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
