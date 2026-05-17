include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

# Board::Board loads assets/mine-sweeper.ttf relative to cwd. Mirror the asset
# tree into the test build directory so tests can run from here without
# depending on the main binary's working directory.
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test_board
    test_board.cpp
    ${CMAKE_SOURCE_DIR}/src/board.cpp
)

target_include_directories(test_board PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_board PRIVATE
    Catch2::Catch2WithMain
    sfml-graphics
    sfml-window
    sfml-system
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_board WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
