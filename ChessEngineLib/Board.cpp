/**
 * @file Board.cpp
 * @author John Korreck
 *
 * Updated implementation with proper check handling
 */

#include "Board.h"
#include "Engine.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

Board::Board(std::string& name, std::string& position) {
    mChessPosition = position;
    mEngine = std::make_shared<Engine>();
    FenParser(mChessPosition);
    GeneratePossibleMoves(false);
}

std::vector<std::vector<int>> Board::FenParser(std::string &fenString) {
    // Clear the board
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            mBoard[i][j] = 0;
        }
    }

    std::stringstream ss(fenString);
    std::string boardPart;
    std::string activeColor;
    std::string castlingRights;
    std::string enPassant;
    std::string halfmove;
    std::string fullmove;

    // Parse FEN components
    ss >> boardPart >> activeColor >> castlingRights >> enPassant >> halfmove >> fullmove;

    // Parse board position
    int rank = 0;
    int file = 0;

    for (char c : boardPart) {
        if (c == '/') {
            rank++;
            file = 0;
        } else if (std::isdigit(c)) {
            file += (c - '0');
        } else {
            int pieceNum = 0;
            switch (c) {
                case 'P': pieceNum = 1; break;
                case 'N': pieceNum = 2; break;
                case 'B': pieceNum = 3; break;
                case 'R': pieceNum = 4; break;
                case 'Q': pieceNum = 5; break;
                case 'K': pieceNum = 6; break;
                case 'p': pieceNum = -1; break;
                case 'n': pieceNum = -2; break;
                case 'b': pieceNum = -3; break;
                case 'r': pieceNum = -4; break;
                case 'q': pieceNum = -5; break;
                case 'k': pieceNum = -6; break;
            }

            if (rank < 8 && file < 8) {
                mBoard[rank][file] = pieceNum;

                if (pieceNum == 6) {
                    char fileChar = 'a' + file;
                    char rankChar = '8' - rank;
                    mWhiteKingSquare = std::string(1, fileChar) + std::string(1, rankChar);
                } else if (pieceNum == -6) {
                    char fileChar = 'a' + file;
                    char rankChar = '8' - rank;
                    mBlackKingSquare = std::string(1, fileChar) + std::string(1, rankChar);
                }
            }
            file++;
        }
    }

    mWhiteTurn = (activeColor == "w");
    mWhiteCastlingKingsideRights = castlingRights.find('K') != std::string::npos;
    mWhiteCastlingQueensideRights = castlingRights.find('Q') != std::string::npos;
    mBlackCastlingKingsideRights = castlingRights.find('k') != std::string::npos;
    mBlackCastlingQueensideRights = castlingRights.find('q') != std::string::npos;
    mEnPassantSquare = (enPassant == "-") ? "" : enPassant;

    // Update check status after parsing FEN
    UpdateCheckStatus();

    return mBoard;
}

std::string Board::GenerateFen() {
    std::string fen = "";

    // Board
    for (int rank = 0; rank < 8; rank++) {
        int emptyCount = 0;
        for (int file = 0; file < 8; file++) {
            int piece = mBoard[rank][file];
            if (piece == 0) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                fen += PieceToString(piece);
            }
        }
        if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
        }
        if (rank < 7) {
            fen += "/";
        }
    }

    // Active color
    fen += mWhiteTurn ? " w " : " b ";

    // Castling rights
    std::string castling = "";
    if (mWhiteCastlingKingsideRights) castling += "K";
    if (mWhiteCastlingQueensideRights) castling += "Q";
    if (mBlackCastlingKingsideRights) castling += "k";
    if (mBlackCastlingQueensideRights) castling += "q";
    fen += castling.empty() ? "-" : castling;
    fen += " ";

    // En passant
    fen += mEnPassantSquare.empty() ? "-" : mEnPassantSquare;

    // Halfmove and fullmove clocks
    fen += " 0 1";

    return fen;
}

bool Board::IsSquareAttacked(const std::string& square, bool byWhite) {
    int targetFile = square[0] - 'a';
    int targetRank = 8 - (square[1] - '0');

    // 1. Check pawn attacks
    int pawnDir = byWhite ? -1 : 1;
    for (int i = 0; i < 2; i++) {
        int fileOffset = (i == 0) ? -1 : 1;
        int file = targetFile + fileOffset;
        int rank = targetRank + pawnDir;
        if (CheckBounds(file, rank)) {
            int piece = mBoard[rank][file];
            if (piece == (byWhite ? 1 : -1)) return true;
        }
    }

    // 2. Check knight attacks
    const int knightMoves[8][2] = {
        {-2,-1}, {-2,1}, {-1,-2}, {-1,2},
        {1,-2}, {1,2}, {2,-1}, {2,1}
    };
    for (int i = 0; i < 8; i++) {
        int file = targetFile + knightMoves[i][0];
        int rank = targetRank + knightMoves[i][1];
        if (CheckBounds(file, rank)) {
            int piece = mBoard[rank][file];
            if (piece == (byWhite ? 2 : -2)) return true;
        }
    }

    // 3. Check king attacks
    const int kingMoves[8][2] = {
        {-1,-1}, {-1,0}, {-1,1}, {0,-1},
        {0,1}, {1,-1}, {1,0}, {1,1}
    };
    for (int i = 0; i < 8; i++) {
        int file = targetFile + kingMoves[i][0];
        int rank = targetRank + kingMoves[i][1];
        if (CheckBounds(file, rank)) {
            int piece = mBoard[rank][file];
            if (piece == (byWhite ? 6 : -6)) return true;
        }
    }

    // 4. Check sliding pieces (bishops, rooks, queens)
    const int bishopDirs[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
    const int rookDirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};

    // Check bishops/queens
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int file = targetFile + bishopDirs[i][0] * dist;
            int rank = targetRank + bishopDirs[i][1] * dist;
            if (!CheckBounds(file, rank)) break;

            int piece = mBoard[rank][file];
            if (piece != 0) {
                if ((byWhite && (piece == 3 || piece == 5)) ||
                    (!byWhite && (piece == -3 || piece == -5))) {
                    return true;
                }
                break;
            }
        }
    }

    // Check rooks/queens
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int file = targetFile + rookDirs[i][0] * dist;
            int rank = targetRank + rookDirs[i][1] * dist;
            if (!CheckBounds(file, rank)) break;

            int piece = mBoard[rank][file];
            if (piece != 0) {
                if ((byWhite && (piece == 4 || piece == 5)) ||
                    (!byWhite && (piece == -4 || piece == -5))) {
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

void Board::GeneratePossibleMoves(bool response) {
    if (response) {
        mResponses.clear();
    } else {
        mPossibleMoves.clear();
    }

    // Check for draw conditions first
    // if (IsDraw()) {
    //     return;
    // }

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int piece = mBoard[rank][file];
            if (piece == 0) continue;

            bool isWhitePiece = piece > 0;
            if (response || (isWhitePiece == mWhiteTurn)) {
                char fileChar = 'a' + file;
                char rankChar = '8' - rank;
                std::string currentSquare = std::string(1, fileChar) + std::string(1, rankChar);

                int pieceType = std::abs(piece);
                switch (pieceType) {
                case 1: GeneratePawnMoves(piece, file, rank, currentSquare, response); break;
                case 2: GenerateKnightMoves(piece, file, rank, currentSquare, response); break;
                case 3: GenerateDiagonalMoves(piece, file, rank, currentSquare, response); break;
                case 4: GenerateSlidingMoves(piece, file, rank, currentSquare, response); break;
                case 5:
                    GenerateSlidingMoves(piece, file, rank, currentSquare, response);
                    GenerateDiagonalMoves(piece, file, rank, currentSquare, response);
                    break;
                case 6: GenerateKingMoves(piece, file, rank, currentSquare, response); break;
                }
            }
        }
    }

    if (!response) {
        std::vector<std::string> legalMoves;
        for (const auto& move : mPossibleMoves) {
            if (IsLegalMove(move)) {
                legalMoves.push_back(move);
            }
        }
        mPossibleMoves = legalMoves;
    }
}

Board::BoardState Board::SaveState() {
    BoardState state;
    state.board = mBoard;
    state.whiteKingSquare = mWhiteKingSquare;
    state.blackKingSquare = mBlackKingSquare;
    state.whiteTurn = mWhiteTurn;
    state.whiteCastlingKingside = mWhiteCastlingKingsideRights;
    state.whiteCastlingQueenside = mWhiteCastlingQueensideRights;
    state.blackCastlingKingside = mBlackCastlingKingsideRights;
    state.blackCastlingQueenside = mBlackCastlingQueensideRights;
    state.enPassant = mEnPassantSquare;
    return state;
}

void Board::RestoreState(const BoardState& state) {
    mBoard = state.board;
    mWhiteKingSquare = state.whiteKingSquare;
    mBlackKingSquare = state.blackKingSquare;
    mWhiteTurn = state.whiteTurn;
    mWhiteCastlingKingsideRights = state.whiteCastlingKingside;
    mWhiteCastlingQueensideRights = state.whiteCastlingQueenside;
    mBlackCastlingKingsideRights = state.blackCastlingKingside;
    mBlackCastlingQueensideRights = state.blackCastlingQueenside;
    mEnPassantSquare = state.enPassant;
}

bool Board::IsLegalMove(const std::string& move) {
    if (move.length() < 4) return false;

    MakeMove(move);
    bool illegal = false;
    GeneratePossibleMoves(true);
    for (std::string response: mResponses)
    {
        std::string responseSquare = "";
        responseSquare += response[2];
        responseSquare += response[3];
        if ((!mWhiteTurn && responseSquare == mWhiteKingSquare) || (mWhiteTurn && responseSquare == mBlackKingSquare))
        {
            illegal = true;
            break;
        }
    }
    UndoMove();

    return !illegal;
}

void Board::UpdateCheckStatus() {
    mWhiteInCheck = false;
    mBlackInCheck = false;

    // Check white king attacks
    std::string kingSquare = mWhiteKingSquare;
    if (IsSquareAttacked(kingSquare, false)) {  // false = by black
        mWhiteInCheck = true;
    }

    // Check black king attacks
    kingSquare = mBlackKingSquare;
    if (IsSquareAttacked(kingSquare, true)) {   // true = by white
        mBlackInCheck = true;
    }
}

void Board::MakeMove(const std::string& move) {
    if (move.length() < 4) return;

    // Save current state to history
    MoveHistory history;
    history.move = move;
    history.whiteCastlingKingside = mWhiteCastlingKingsideRights;
    history.whiteCastlingQueenside = mWhiteCastlingQueensideRights;
    history.blackCastlingKingside = mBlackCastlingKingsideRights;
    history.blackCastlingQueenside = mBlackCastlingQueensideRights;
    history.enPassantSquare = mEnPassantSquare;
    history.whiteKingSquare = mWhiteKingSquare;
    history.blackKingSquare = mBlackKingSquare;
    history.halfMoveClock = mHalfMoveClock;
    history.fullMoveNumber = mFullMoveNumber;

    int fromFile = move[0] - 'a';
    int fromRank = 8 - (move[1] - '0');
    int toFile = move[2] - 'a';
    int toRank = 8 - (move[3] - '0');

    // Handle capture (save captured piece)
    history.capturedPiece = mBoard[toRank][toFile];
    if (history.capturedPiece != 0 || (abs(mBoard[fromRank][fromFile]) == 1 && toFile != fromFile)) {
        mHalfMoveClock = 0; // Reset on capture or pawn move
    } else {
        mHalfMoveClock++;
    }

    // Move the piece
    int piece = mBoard[fromRank][fromFile];
    mBoard[fromRank][fromFile] = 0;
    mBoard[toRank][toFile] = piece;

    // Handle special moves
    if (abs(piece) == 6) { // King moved
        // Update king position
        std::string newSquare = move.substr(2, 2);
        if (piece > 0) {
            mWhiteKingSquare = newSquare;
        } else {
            mBlackKingSquare = newSquare;
        }

        // Remove castling rights
        if (piece > 0) {
            mWhiteCastlingKingsideRights = false;
            mWhiteCastlingQueensideRights = false;
        } else {
            mBlackCastlingKingsideRights = false;
            mBlackCastlingQueensideRights = false;
        }

        // Handle castling
        if (abs(fromFile - toFile) == 2) { // Castling move
            int rookFromFile = (toFile > fromFile) ? 7 : 0;
            int rookToFile = (toFile > fromFile) ? 5 : 3;
            int rookRank = (piece > 0) ? 7 : 0;

            // Move the rook
            int rookPiece = mBoard[rookRank][rookFromFile];
            mBoard[rookRank][rookFromFile] = 0;
            mBoard[rookRank][rookToFile] = rookPiece;
        }
    }
    else if (abs(piece) == 4) { // Rook moved
        // Remove castling rights if rook moves from starting position
        if (piece > 0) {
            if (fromRank == 7 && fromFile == 0) mWhiteCastlingQueensideRights = false;
            if (fromRank == 7 && fromFile == 7) mWhiteCastlingKingsideRights = false;
        } else {
            if (fromRank == 0 && fromFile == 0) mBlackCastlingQueensideRights = false;
            if (fromRank == 0 && fromFile == 7) mBlackCastlingKingsideRights = false;
        }
    }

    // Handle en passant
    mEnPassantSquare = "";
    if (abs(piece) == 1 && abs(fromRank - toRank) == 2) {
        int epRank = (piece > 0) ? toRank + 1 : toRank - 1;
        mEnPassantSquare = move.substr(0, 1) + std::to_string(8 - epRank);
    }
    else if (abs(piece) == 1 && !history.enPassantSquare.empty() &&
             toFile == (history.enPassantSquare[0] - 'a') &&
             toRank == (8 - (history.enPassantSquare[1] - '0'))) {
        // Handle en passant capture
        int capturedPawnRank = (piece > 0) ? toRank + 1 : toRank - 1;
        history.capturedPiece = mBoard[capturedPawnRank][toFile];
        mBoard[capturedPawnRank][toFile] = 0;
    }

    // Handle promotion (auto-queen)
    if (abs(piece) == 1 && (toRank == 0 || toRank == 7)) {
        int promoPiece = (piece > 0) ? 5 : -5; // Auto-queen
        mBoard[toRank][toFile] = promoPiece;
    }

    // Update move counters
    if (!mWhiteTurn) {
        mFullMoveNumber++;
    }

    mWhiteTurn = !mWhiteTurn;
    mChessPosition = GenerateFen();
    mHistory.push_back(history);
    mPositionHistory[mChessPosition]++;
}

void Board::UndoMove() {
    if (mHistory.empty()) return;

    const MoveHistory& history = mHistory.back();
    std::string move = history.move;

    int fromFile = move[0] - 'a';
    int fromRank = 8 - (move[1] - '0');
    int toFile = move[2] - 'a';
    int toRank = 8 - (move[3] - '0');

    // Move piece back
    int piece = mBoard[toRank][toFile];
    mBoard[fromRank][fromFile] = piece;
    mBoard[toRank][toFile] = history.capturedPiece;

    // Restore king position
    if (abs(piece) == 6) {
        if (piece > 0) {
            mWhiteKingSquare = history.whiteKingSquare;
        } else {
            mBlackKingSquare = history.blackKingSquare;
        }

        // Handle castling undo
        if (abs(fromFile - toFile) == 2) {
            int rookFromFile = (toFile > fromFile) ? 7 : 0;
            int rookToFile = (toFile > fromFile) ? 5 : 3;
            int rookRank = (piece > 0) ? 7 : 0;

            // Move rook back
            int rookPiece = mBoard[rookRank][rookToFile];
            mBoard[rookRank][rookToFile] = 0;
            mBoard[rookRank][rookFromFile] = rookPiece;
        }
    }

    // Handle en passant undo
    if (abs(piece) == 1 && !history.enPassantSquare.empty() &&
        toFile == (history.enPassantSquare[0] - 'a') &&
        toRank == (8 - (history.enPassantSquare[1] - '0'))) {
        int capturedPawnRank = (piece > 0) ? toRank + 1 : toRank - 1;
        mBoard[capturedPawnRank][toFile] = history.capturedPiece;
    }

    // Restore state
    mWhiteCastlingKingsideRights = history.whiteCastlingKingside;
    mWhiteCastlingQueensideRights = history.whiteCastlingQueenside;
    mBlackCastlingKingsideRights = history.blackCastlingKingside;
    mBlackCastlingQueensideRights = history.blackCastlingQueenside;
    mEnPassantSquare = history.enPassantSquare;
    mWhiteTurn = !mWhiteTurn;
    mChessPosition = GenerateFen();

    mHistory.pop_back();
}

// int Board::CountMoves(int depth) {
//     if (depth == 0) return 1;
//
//     int count = 0;
//     GeneratePossibleMoves(false);
//
//     for (const auto& move : mPossibleMoves) {
//         MakeMove(move);
//         count += CountMoves(depth - 1);
//         UndoMove();
//     }
//
//     return count;
// }

bool Board::IsPinned(int file, int rank) {
    BoardState savedState = SaveState();
    int piece = mBoard[rank][file];
    mBoard[rank][file] = 0; // Temporarily remove piece

    bool inCheck = (piece > 0) ?
        IsSquareAttacked(mWhiteKingSquare, false) :
        IsSquareAttacked(mBlackKingSquare, true);

    RestoreState(savedState);
    return inCheck;
}

bool Board::CanCastle(bool kingside, bool white) {
    if (white) {
        if (kingside && !mWhiteCastlingKingsideRights) return false;
        if (!kingside && !mWhiteCastlingQueensideRights) return false;
    } else {
        if (kingside && !mBlackCastlingKingsideRights) return false;
        if (!kingside && !mBlackCastlingQueensideRights) return false;
    }

    int rank = white ? 7 : 0;
    std::string kingPos = white ? mWhiteKingSquare : mBlackKingSquare;
    int kingFile = kingPos[0] - 'a';

    // Check squares between king and rook aren't under attack
    for (int file = kingside ? kingFile+1 : kingFile-1;
         file != (kingside ? 7 : 0);
         kingside ? file++ : file--) {
        std::string square = std::string(1, 'a'+file) + std::to_string(8-rank);
        if (mBoard[rank][file] != 0 || IsSquareAttacked(square, !white)) {
            return false;
        }
    }
    return true;
}

// bool Board::IsDraw() {
//     // 50-move rule
//     if (mHalfMoveClock >= 50) return true;
//
//     // Threefold repetition
//     if (mPositionHistory[mChessPosition] >= 3) return true;
//
//     // Insufficient material
//     int whitePieces = 0, blackPieces = 0;
//     bool whiteHasNonKing = false, blackHasNonKing = false;
//     bool whiteHasPawnOrMajor = false, blackHasPawnOrMajor = false;
//
//     for (int rank = 0; rank < 8; rank++) {
//         for (int file = 0; file < 8; file++) {
//             int piece = mBoard[rank][file];
//             if (piece == 0) continue;
//
//             if (piece > 0) {
//                 whitePieces++;
//                 if (abs(piece) != 6) whiteHasNonKing = true;
//                 if (abs(piece) == 1 || abs(piece) >= 4) whiteHasPawnOrMajor = true;
//             } else {
//                 blackPieces++;
//                 if (abs(piece) != 6) blackHasNonKing = true;
//                 if (abs(piece) == 1 || abs(piece) >= 4) blackHasPawnOrMajor = true;
//             }
//         }
//     }
//
//     // King vs King
//     if (!whiteHasNonKing && !blackHasNonKing) return true;
//
//     // King + minor vs King
//     if ((whitePieces == 1 && !blackHasPawnOrMajor && blackPieces <= 2) ||
//         (blackPieces == 1 && !whiteHasPawnOrMajor && whitePieces <= 2)) {
//         return true;
//         }
//
//     return false;
// }

void Board::GenerateSlidingMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    // Rook moves (horizontal and vertical)
    int directions[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    for (int dir = 0; dir < 4; dir++) {
        int newFile = file;
        int newRank = rank;

        while (true) {
            newFile += directions[dir][0];
            newRank += directions[dir][1];

            if (!CheckBounds(newFile, newRank)) break;

            int targetPiece = mBoard[newRank][newFile];

            // If square is empty, add move
            if (targetPiece == 0) {
                char fileChar = 'a' + newFile;
                char rankChar = '8' - newRank;
                std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                std::string move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
            }
            // If square has opponent's piece, add capture and stop
            else if ((pieceNum > 0 && targetPiece < 0) || (pieceNum < 0 && targetPiece > 0)) {
                char fileChar = 'a' + newFile;
                char rankChar = '8' - newRank;
                std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                std::string move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
                break;
            }
            // If square has own piece, stop
            else {
                break;
            }
        }
    }
}

void Board::GenerateDiagonalMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    // Bishop moves (diagonal)
    int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (int dir = 0; dir < 4; dir++) {
        int newFile = file;
        int newRank = rank;

        while (true) {
            newFile += directions[dir][0];
            newRank += directions[dir][1];

            if (!CheckBounds(newFile, newRank)) break;

            int targetPiece = mBoard[newRank][newFile];

            // If square is empty, add move
            if (targetPiece == 0) {
                char fileChar = 'a' + newFile;
                char rankChar = '8' - newRank;
                std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                std::string move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
            }
            // If square has opponent's piece, add capture and stop
            else if ((pieceNum > 0 && targetPiece < 0) || (pieceNum < 0 && targetPiece > 0)) {
                char fileChar = 'a' + newFile;
                char rankChar = '8' - newRank;
                std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                std::string move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
                break;
            }
            // If square has own piece, stop
            else {
                break;
            }
        }
    }
}

bool Board::CheckBounds(int file, int rank) {
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

void Board::GenerateKingMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    for (int dir = 0; dir < 8; dir++) {
        int newFile = file + directions[dir][0];
        int newRank = rank + directions[dir][1];

        if (!CheckBounds(newFile, newRank)) continue;

        int targetPiece = mBoard[newRank][newFile];

        // Can move to empty square or capture opponent's piece
        if (targetPiece == 0 || (pieceNum > 0 && targetPiece < 0) || (pieceNum < 0 && targetPiece > 0)) {
            // Temporarily make move to check for safety
            BoardState savedState = SaveState();
            mBoard[rank][file] = 0;
            mBoard[newRank][newFile] = pieceNum;

            std::string newKingSquare = std::string(1, 'a'+newFile) + std::to_string(8-newRank);
            bool safe = !IsSquareAttacked(newKingSquare, pieceNum < 0);

            RestoreState(savedState);

            if (safe) {
                char fileChar = 'a' + newFile;
                char rankChar = '8' - newRank;
                std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                std::string move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
            }
        }
    }

    // Castling
    if (!response) {
        bool isWhite = pieceNum > 0;
        if (isWhite == mWhiteTurn) {
            // Kingside
            if ((isWhite && mWhiteCastlingKingsideRights) || (!isWhite && mBlackCastlingKingsideRights)) {
                if (CanCastle(true, isWhite)) {
                    mPossibleMoves.push_back(isWhite ? "e1g1" : "e8g8");
                }
            }
            // Queenside
            if ((isWhite && mWhiteCastlingQueensideRights) || (!isWhite && mBlackCastlingQueensideRights)) {
                if (CanCastle(false, isWhite)) {
                    mPossibleMoves.push_back(isWhite ? "e1c1" : "e8c8");
                }
            }
        }
    }
}

void Board::GeneratePawnMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    int direction = (pieceNum > 0) ? -1 : 1; // White pawns move up (-1), black pawns move down (+1)
    int startRank = (pieceNum > 0) ? 6 : 1;

    // Forward move
    int newRank = rank + direction;
    if (CheckBounds(file, newRank) && mBoard[newRank][file] == 0) {
        char fileChar = 'a' + file;
        char rankChar = '8' - newRank;
        std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
        std::string move = currentSquare + targetSquare;

        if (response) {
            mResponses.push_back(move);
        } else {
            mPossibleMoves.push_back(move);
        }

        // Double move from starting position
        if (rank == startRank) {
            newRank = rank + 2 * direction;
            if (CheckBounds(file, newRank) && mBoard[newRank][file] == 0) {
                rankChar = '8' - newRank;
                targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
            }
        }
    }

    // En passant (fixed and moved outside forward move block)
    if (!mEnPassantSquare.empty() && !response) {
        int epFile = mEnPassantSquare[0] - 'a';
        int epRank = '8' - mEnPassantSquare[1];

        // Check if we're on the correct rank (5th for white, 4th for black)
        bool correctRank = (pieceNum > 0) ? (rank == 3) : (rank == 4);

        if (correctRank && (file == epFile - 1 || file == epFile + 1)) {
            // The target square is one rank forward in the en passant file
            int targetRank = rank + direction;
            char fileChar = mEnPassantSquare[0];
            char rankChar = '8' - targetRank;
            std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
            std::string move = currentSquare + targetSquare;

            mPossibleMoves.push_back(move);
        }
    }

    // Capture moves (unchanged)
    int captureFiles[2] = {file - 1, file + 1};
    for (int i = 0; i < 2; i++) {
        int newFile = captureFiles[i];
        newRank = rank + direction;

        if (CheckBounds(newFile, newRank)) {
            int targetPiece = mBoard[newRank][newFile];

            // Can capture opponent's piece
            if ((pieceNum > 0 && targetPiece < 0) || (pieceNum < 0 && targetPiece > 0)) {
                char fileChar = 'a' + newFile;
                char rankChar = '8' - newRank;
                std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
                std::string move = currentSquare + targetSquare;

                if (response) {
                    mResponses.push_back(move);
                } else {
                    mPossibleMoves.push_back(move);
                }
            }
        }
    }
}

void Board::GenerateKnightMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

    for (int i = 0; i < 8; i++) {
        int newFile = file + knightMoves[i][0];
        int newRank = rank + knightMoves[i][1];

        if (!CheckBounds(newFile, newRank)) continue;

        int targetPiece = mBoard[newRank][newFile];

        // Can move to empty square or capture opponent's piece
        if (targetPiece == 0 || (pieceNum > 0 && targetPiece < 0) || (pieceNum < 0 && targetPiece > 0)) {
            char fileChar = 'a' + newFile;
            char rankChar = '8' - newRank;
            std::string targetSquare = std::string(1, fileChar) + std::string(1, rankChar);
            std::string move = currentSquare + targetSquare;

            if (response) {
                mResponses.push_back(move);
            } else {
                mPossibleMoves.push_back(move);
            }
        }
    }
}

void Board::displayWinner() {
    GeneratePossibleMoves(false);

    if (mPossibleMoves.empty()) {
        if (mWhiteTurn) {
            if (mWhiteInCheck) {
                std::cout << "Checkmate! Black wins!" << std::endl;
            } else {
                std::cout << "Stalemate! It's a draw!" << std::endl;
            }
        } else {
            if (mBlackInCheck) {
                std::cout << "Checkmate! White wins!" << std::endl;
            } else {
                std::cout << "Stalemate! It's a draw!" << std::endl;
            }
        }
    }
}

std::string Board::PieceToString(int pieceNum) {
    switch (pieceNum) {
        case 1: return "P";
        case 2: return "N";
        case 3: return "B";
        case 4: return "R";
        case 5: return "Q";
        case 6: return "K";
        case -1: return "p";
        case -2: return "n";
        case -3: return "b";
        case -4: return "r";
        case -5: return "q";
        case -6: return "k";
        default: return " ";
    }
}

void Board::PrintInternalBoard() {
    std::cout << "  a b c d e f g h" << std::endl;
    for (int rank = 0; rank < 8; rank++) {
        std::cout << (8 - rank) << " ";
        for (int file = 0; file < 8; file++) {
            std::cout << PieceToString(mBoard[rank][file]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Board::Play() {

    std::cout << "Option 1 - Human vs Human" << std::endl << "Option 2 - Human vs Engine" << std::endl << "Select 1 or 2: ";
    int gameSelection;

    std::cin >> gameSelection;

    while (gameSelection != 1 && gameSelection != 2)
    {
        std::cout << "Invalid Option" << std::endl;
        std::cout << "Select 1 or 2: ";
        std::cin >> gameSelection;
    }

    if (gameSelection == 1)
    {
        PlayHuman();
    }

    else if (gameSelection == 2)
    {
        PlayEngine();
    }
}

void Board::PlayHuman()
{
    std::string userMove;

    while (true) {
        PrintInternalBoard();

        std::cout << "Current position: " << mChessPosition << std::endl;
        std::cout << (mWhiteTurn ? "White" : "Black") << " to move." << std::endl;

        GeneratePossibleMoves(false);

        if (mPossibleMoves.empty()) {
            displayWinner();
            break;
        }

        std::cout << "Possible moves: ";
        for (const auto& move : mPossibleMoves) {
            std::cout << move << " ";
        }
        std::cout << std::endl;

        std::cout << "Enter your move (e.g., e2e4): ";
        std::cin >> userMove;

        // Check if move is valid
        bool validMove = false;
        for (const auto& move : mPossibleMoves) {
            if (move == userMove) {
                validMove = true;
                break;
            }
        }

        if (validMove) {
            MakeMove(userMove);
        } else {
            std::cout << "Invalid move! Please try again." << std::endl;
        }
    }
}

void Board::PlayEngine() {
    std::string userMove;

    while (true) {
        GeneratePossibleMoves(false);

        if (mPossibleMoves.empty()) {
            displayWinner();
            break;
        }

        if (mWhiteTurn) {
            // Player's turn
            PrintInternalBoard();
            std::cout << "Current position: " << mChessPosition << std::endl;
            std::cout << "Possible moves: ";
            for (const auto& move : mPossibleMoves) {
                std::cout << move << " ";
            }
            std::cout << std::endl;
            while (true) {
                std::cout << "Enter your move: ";
                std::cin >> userMove;

                if (std::find(mPossibleMoves.begin(), mPossibleMoves.end(), userMove) != mPossibleMoves.end()) {
                    MakeMove(userMove);
                    std::cout << "Made move: " << userMove << std::endl;
                    break;
                }
                std::cout << "Invalid move! Try again." << std::endl;
            }
        } else {
            // Engine's turn
            std::string engineMove = mEngine->FindBestMove(*this, 3);
            std::cout << "Engine moving: " << engineMove << std::endl;
            MakeMove(engineMove);
            std::cout << "Engine made move: " << engineMove << std::endl;
        }
    }
}