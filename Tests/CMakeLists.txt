cmake_minimum_required(VERSION 3.16)

# Disable precompiled headers for tests
set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0  # Verified working tag
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(Tests_run
        gtest_main.cpp
        MoveGenerationTest.cpp
        DifficultMoveGenerationTest.cpp
)

target_link_libraries(Tests_run
        PRIVATE
        ChessEngineLib
        GTest::gtest_main
)

target_include_directories(Tests_run
        PRIVATE
        ${CMAKE_SOURCE_DIR}/ChessEngineLib
)

enable_testing()
add_test(NAME MoveGenerationTest COMMAND Tests_run)