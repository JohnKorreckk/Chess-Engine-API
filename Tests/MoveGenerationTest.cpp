#include "gtest/gtest.h"
#include "Board.h"

static int countMoves(Board &board, int depth) {
    if (depth == 0) return 1;

    int count = 0;
    board.GeneratePossibleMoves(false);
    auto moves = board.GetPossibleMoves();
    for (const auto& move : moves) {
        board.MakeMove(move);
        count += countMoves(board, depth - 1);
        board.UndoMove();
    }
    return count;
}

// Initial positions
TEST(MoveGenerationTest, InitialPositionDepth1) {
    std::string name = "Board";
    std::string position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board board(name, position);

    int count = countMoves(board, 1);
    std::cout << "Count " << count << std::endl;

    // Test depth 1 (should be 20 moves for initial position)
    EXPECT_EQ(count, 20);
}

TEST(MoveGenerationTest, InitialPositionDepth2) {
    std::string name = "Board";
    std::string position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board board(name, position);

    int count = countMoves(board, 2);
    std::cout << "Count " << count << std::endl;

    // Test depth 2 (should be 400 moves for initial position)
    EXPECT_EQ(count, 400);
}

TEST(MoveGenerationTest, InitialPositionDepth3) {
    std::string name = "Board";
    std::string position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board board(name, position);

    int count = countMoves(board, 3);
    std::cout << "Count " << count << std::endl;

    // Test depth 4 (should be 400 moves for initial position)
    EXPECT_EQ(count, 8902);
}