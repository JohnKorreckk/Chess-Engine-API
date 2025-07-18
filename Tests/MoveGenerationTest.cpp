#include "gtest/gtest.h"
#include "Board.h"

int countMoves(Board &board, int depth) {
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


// Diffuclt positions
TEST(MoveGenerationTest, DifficultPositionDepth1) {
    std::string name = "Board";
    std::string position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
    Board board(name, position);

    int count = countMoves(board, 1);
    std::cout << "Count " << count << std::endl;

    EXPECT_EQ(count, 48);
}

TEST(MoveGenerationTest, DifficultPositionDepth2) {
    std::string name = "Board";
    std::string position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
    Board board(name, position);

    int count = countMoves(board, 2);
    std::cout << "Count " << count << std::endl;

    EXPECT_EQ(count, 2039);
}

TEST(MoveGenerationTest, DifficultPositionDepth3) {
    std::string name = "Board";
    std::string position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
    Board board(name, position);

    int count = countMoves(board, 3);
    std::cout << "Count " << count << std::endl;

    EXPECT_EQ(count, 97862);
}