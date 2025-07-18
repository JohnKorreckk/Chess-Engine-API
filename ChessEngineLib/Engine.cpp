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

// Evaluate board based on material count
int Engine::EvaluateBoard(Board& board)
{
    std::vector<std::vector<int>> boardArray = board.GetBoard();
    int eval = 0;
    for (int rank = 0; rank < boardArray.size(); rank++)
    {
        for (int file = 0; file < boardArray[0].size(); file++)
        {
            switch (boardArray[rank][file])
            {
                case WHITE_PAWN: eval += 1; break;
                case WHITE_KNIGHT: eval += 3; break;
                case WHITE_BISHOP: eval += 3; break;
                case WHITE_ROOK: eval += 5; break;
                case WHITE_QUEEN: eval += 9; break;

                case BLACK_PAWN: eval -= 1; break;
                case BLACK_KNIGHT: eval -= 3; break;
                case BLACK_BISHOP: eval -= 3; break;
                case BLACK_ROOK: eval -= 5; break;
                case BLACK_QUEEN: eval -= 9; break;

                default: break;
            }
        }
    }
    return eval;
}

// Remove these from Engine.h/Engine.cpp entirely:
// void MakeMove(Board& board, const std::string& move);
// void UndoMove(Board& board);

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
        // Checkmate or stalemate
        return EvaluateBoard(board); // TODO: Add proper checkmate detection
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : possibleMoves) {
            Board tempBoard = board; // Create copy
            tempBoard.MakeMove(move); // Use Board's move function
            int eval = Minimax(tempBoard, depth - 1, false, alpha, beta);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break; // Beta cutoff
        }
        return maxEval;
    }
    else { // Minimizing player
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : possibleMoves) {
            Board tempBoard = board; // Create copy
            tempBoard.MakeMove(move); // Use Board's move function
            int eval = Minimax(tempBoard, depth - 1, true, alpha, beta);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break; // Alpha cutoff
        }
        return minEval;
    }
}