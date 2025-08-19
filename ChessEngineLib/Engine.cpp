/**
 * @file Engine.cpp
 * @author John Korreck
 */
 
#include "Engine.h"
#include "Board.h"

#include <limits>

// Piece definitions (same as your current code)
const int EMPTY = 0;
const int KING = 1;
const int PAWN = 2;
const int KNIGHT = 3;
const int BISHOP = 4;
const int ROOK = 5;
const int QUEEN = 6;

const int WHITE = 8;
const int BLACK = 16;

const int WHITE_KING = WHITE + KING;
const int WHITE_PAWN = WHITE + PAWN;
const int WHITE_KNIGHT = WHITE + KNIGHT;
const int WHITE_BISHOP = WHITE + BISHOP;
const int WHITE_ROOK = WHITE + ROOK;
const int WHITE_QUEEN = WHITE + QUEEN;

const int BLACK_KING = BLACK + KING;
const int BLACK_PAWN = BLACK + PAWN;
const int BLACK_KNIGHT = BLACK + KNIGHT;
const int BLACK_BISHOP = BLACK + BISHOP;
const int BLACK_ROOK = BLACK + ROOK;
const int BLACK_QUEEN = BLACK + QUEEN;

int Engine::EvaluateBoard(Board& board) {
    std::vector<std::vector<int>> boardArray = board.GetBoard();
    int materialEval = 0;
    int controlEval = 0;

    // Piece square tables for positional evaluation
    const int pawnTable[8][8] = {
        { 0,  0,  0,  0,  0,  0,  0,  0},
        {50, 50, 50, 50, 50, 50, 50, 50},
        {10, 10, 20, 30, 30, 20, 10, 10},
        { 5,  5, 10, 25, 25, 10,  5,  5},
        { 0,  0,  0, 20, 20,  0,  0,  0},
        { 5, -5,-10,  0,  0,-10, -5,  5},
        { 5, 10, 10,-20,-20, 10, 10,  5},
        { 0,  0,  0,  0,  0,  0,  0,  0}
    };

    const int knightTable[8][8] = {
        {-50,-40,-30,-30,-30,-30,-40,-50},
        {-40,-20,  0,  0,  0,  0,-20,-40},
        {-30,  0, 10, 15, 15, 10,  0,-30},
        {-30,  5, 15, 20, 20, 15,  5,-30},
        {-30,  0, 15, 20, 20, 15,  0,-30},
        {-30,  5, 10, 15, 15, 10,  5,-30},
        {-40,-20,  0,  5,  5,  0,-20,-40},
        {-50,-40,-30,-30,-30,-30,-40,-50}
    };

    const int bishopTable[8][8] = {
        {-20,-10,-10,-10,-10,-10,-10,-20},
        {-10,  0,  0,  0,  0,  0,  0,-10},
        {-10,  0,  5, 10, 10,  5,  0,-10},
        {-10,  5,  5, 10, 10,  5,  5,-10},
        {-10,  0, 10, 10, 10, 10,  0,-10},
        {-10, 10, 10, 10, 10, 10, 10,-10},
        {-10,  5,  0,  0,  0,  0,  5,-10},
        {-20,-10,-10,-10,-10,-10,-10,-20}
    };

    // Central control bonus (squares e4,e5,d4,d5)
    const int centralControlBonus = 10;

    for (int rank = 0; rank < boardArray.size(); rank++) {
        for (int file = 0; file < boardArray[0].size(); file++) {
            int piece = boardArray[rank][file];
            if (piece == EMPTY) continue;

            bool isWhite = (piece & WHITE);
            int sign = isWhite ? 1 : -1;
            int pieceType = piece & 7; // Get piece type without color

            // Material evaluation
            switch (pieceType) {
                case PAWN:
                    materialEval += sign * 100;
                    materialEval += sign * pawnTable[isWhite ? rank : 7-rank][file];
                    break;
                case KNIGHT:
                    materialEval += sign * 300;
                    materialEval += sign * knightTable[isWhite ? rank : 7-rank][file];
                    break;
                case BISHOP:
                    materialEval += sign * 300;
                    materialEval += sign * bishopTable[isWhite ? rank : 7-rank][file];
                    break;
                case ROOK: materialEval += sign * 500; break;
                case QUEEN: materialEval += sign * 900; break;
            }

            // Control evaluation
            int controlValue = 0;
            switch (pieceType) {
                case PAWN: {
                    // Pawns control diagonally forward
                    int forward = isWhite ? -1 : 1;
                    for (int i = -1; i <= 1; i += 2) {
                        int newFile = file + i;
                        int newRank = rank + forward;
                        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
                            controlValue += 5; // Base control value
                            // Bonus for controlling center
                            if ((newFile >= 3 && newFile <= 4) && (newRank >= 3 && newRank <= 4)) {
                                controlValue += centralControlBonus;
                            }
                        }
                    }
                    break;
                }
                case KNIGHT: {
                    // Knight moves
                    const int knightMoves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
                    for (const auto& move : knightMoves) {
                        int newFile = file + move[0];
                        int newRank = rank + move[1];
                        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
                            controlValue += 10;
                            if ((newFile >= 3 && newFile <= 4) && (newRank >= 3 && newRank <= 4)) {
                                controlValue += centralControlBonus * 2;
                            }
                        }
                    }
                    break;
                }
                case BISHOP:
                case ROOK:
                case QUEEN: {
                    // Sliding pieces
                    const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
                    const int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                    const int* dirs = (pieceType == BISHOP) ? *bishopDirs :
                                      (pieceType == ROOK) ? *rookDirs : nullptr;
                    int dirCount = (pieceType == BISHOP) ? 4 :
                                  (pieceType == ROOK) ? 4 : 8;

                    if (pieceType == QUEEN) {
                        // Queen combines bishop and rook
                        for (int i = 0; i < 8; i++) {
                            const int* dir = (i < 4) ? bishopDirs[i] : rookDirs[i-4];
                            for (int dist = 1; dist < 8; dist++) {
                                int newFile = file + dir[0] * dist;
                                int newRank = rank + dir[1] * dist;
                                if (newFile < 0 || newFile >= 8 || newRank < 0 || newRank >= 8) break;
                                controlValue += 5;
                                if ((newFile >= 3 && newFile <= 4) && (newRank >= 3 && newRank <= 4)) {
                                    controlValue += centralControlBonus;
                                }
                                if (boardArray[newRank][newFile] != EMPTY) break; // Blocked
                            }
                        }
                    } else {
                        for (int i = 0; i < dirCount; i++) {
                            const int* dir = (pieceType == BISHOP) ? bishopDirs[i] : rookDirs[i];
                            for (int dist = 1; dist < 8; dist++) {
                                int newFile = file + dir[0] * dist;
                                int newRank = rank + dir[1] * dist;
                                if (newFile < 0 || newFile >= 8 || newRank < 0 || newRank >= 8) break;
                                controlValue += 5;
                                if ((newFile >= 3 && newFile <= 4) && (newRank >= 3 && newRank <= 4)) {
                                    controlValue += centralControlBonus;
                                }
                                if (boardArray[newRank][newFile] != EMPTY) break; // Blocked
                            }
                        }
                    }
                    break;
                }
                case KING: {
                    // King control (less important)
                    const int kingMoves[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
                    for (const auto& move : kingMoves) {
                        int newFile = file + move[0];
                        int newRank = rank + move[1];
                        if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
                            controlValue += 3; // King control is less valuable
                        }
                    }
                    break;
                }
            }

            controlEval += sign * controlValue;
        }
    }

    // Combine material and control evaluations with weights
    return materialEval + (controlEval / 5); // Adjust weight as needed
}

// Modify FindBestMove to use board copies:
std::string Engine::FindBestMove(Board& board, int depth) {
    std::string bestMove;
    int bestEval = std::numeric_limits<int>::min();

    std::vector<std::string> possibleMoves = board.GetPossibleMoves();

    for (const auto& move : possibleMoves) {
        // Create a copy of the board for simulation
        Board tempBoard = board;
        tempBoard.MakeMove(move);  // Use Board's move function

        int eval = Minimax(tempBoard, depth - 1, false,
                         std::numeric_limits<int>::min(),
                         std::numeric_limits<int>::max());

        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
        }
    }
    return bestMove;
}

int Engine::Minimax(Board& board, int depth, bool maximizingPlayer, int alpha, int beta) {
    if (depth == 0) {
        return EvaluateBoard(board);
    }

    board.GeneratePossibleMoves(false);
    std::vector<std::string> possibleMoves = board.GetPossibleMoves();

    if (possibleMoves.empty()) {
        bool inCheck = board.IsWhiteTurn() ? board.IsWhiteInCheck() : board.IsBlackInCheck();

            return maximizingPlayer ? std::numeric_limits<int>::min()
                                    : std::numeric_limits<int>::max();
        }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : possibleMoves) {
            Board tempBoard = board;
            tempBoard.MakeMove(move);
            int eval = Minimax(tempBoard, depth - 1, false, alpha, beta);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break; // Beta cutoff
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : possibleMoves) {
            Board tempBoard = board;
            tempBoard.MakeMove(move);
            int eval = Minimax(tempBoard, depth - 1, true, alpha, beta);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break; // Alpha cutoff
        }
        return minEval;
    }
}