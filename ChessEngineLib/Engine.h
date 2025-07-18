/**
* @file Engine.h
 * @author John Korreck
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>

class Board;

struct MoveData
{
 int fromRank, fromFile;
 int toRank, toFile;
 int movedPiece;
 int capturedPiece;
 int previousCastlingRights;
 int previousEnPassantSquare;
 int previousHalfMoveClock;
 int previousFullMoveNumber;
};

class Engine {
private:
 std::vector<MoveData> moveHistory;

 int castlingRights = 0;
 int enPassantSquare = -1; // -1 if no en passant available
 int halfMoveClock = 0;
 int fullMoveNumber = 1;

public:
 std::string FindBestMove(Board& board, int depth);
 int Minimax(Board& board, int depth, bool maximizingPlayer, int alpha, int beta);
 int EvaluateBoard(Board& board);
};

#endif //ENGINE_H
