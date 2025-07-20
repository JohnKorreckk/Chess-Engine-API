/**
 * @file DifficultMoveGenerationTest.cpp
 * @author John Korreck
 */

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


// Difficult positions
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