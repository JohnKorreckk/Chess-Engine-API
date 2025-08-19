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

    // Pawns
    int pawnDir = byWhite ? 1 : -1; // white pawns attack upwards (rank -1), so they come from below
    int pawnPiece = byWhite ? 1 : -1;
    for (int i = 0; i < 2; i++) {
        int file = targetFile + (i == 0 ? -1 : 1);
        int rank = targetRank + pawnDir;
        if (CheckBounds(file, rank)) {
            int piece = mBoard[rank][file];
            if (piece == pawnPiece)
                return true;
        }
    }

    // Knights
    static const int knightMoves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (const auto& move : knightMoves) {
        int file = targetFile + move[0];
        int rank = targetRank + move[1];
        if (CheckBounds(file, rank) && mBoard[rank][file] == (byWhite ? 2 : -2))
            return true;
    }

    // King (only need to check adjacent squares)
    static const int kingMoves[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (const auto& move : kingMoves) {
        int file = targetFile + move[0];
        int rank = targetRank + move[1];
        if (CheckBounds(file, rank) && mBoard[rank][file] == (byWhite ? 6 : -6))
            return true;
    }

    // Sliding pieces
    static const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    static const int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    // Check bishops/queens
    for (const auto& dir : bishopDirs) {
        for (int dist = 1; dist < 8; dist++) {
            int file = targetFile + dir[0] * dist;
            int rank = targetRank + dir[1] * dist;
            if (!CheckBounds(file, rank)) break;

            int piece = mBoard[rank][file];
            if (piece != 0) {
                if ((byWhite && (piece == 3 || piece == 5)) ||
                    (!byWhite && (piece == -3 || piece == -5)))
                    return true;
                break;
            }
        }
    }

    // Check rooks/queens
    for (const auto& dir : rookDirs) {
        for (int dist = 1; dist < 8; dist++) {
            int file = targetFile + dir[0] * dist;
            int rank = targetRank + dir[1] * dist;
            if (!CheckBounds(file, rank)) break;

            int piece = mBoard[rank][file];
            if (piece != 0) {
                if ((byWhite && (piece == 4 || piece == 5)) ||
                    (!byWhite && (piece == -4 || piece == -5)))
                    return true;
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

    // Save state
    BoardState savedState = SaveState();

    // Make the move temporarily
    int fromFile = move[0] - 'a';
    int fromRank = 8 - (move[1] - '0');
    int toFile = move[2] - 'a';
    int toRank = 8 - (move[3] - '0');
    int piece = mBoard[fromRank][fromFile];
    int captured = mBoard[toRank][toFile];

    // Special case: en passant
    if (abs(piece) == 1 && !mEnPassantSquare.empty() &&
        toFile == (mEnPassantSquare[0] - 'a') &&
        toRank == (8 - (mEnPassantSquare[1] - '0'))) {
        captured = mBoard[fromRank][toFile]; // Captured pawn is on same file
        }

    // Make the move on board
    mBoard[fromRank][fromFile] = 0;
    mBoard[toRank][toFile] = piece;

    // Update king position if moving king
    std::string kingSquare;
    if (abs(piece) == 6) {
        kingSquare = move.substr(2, 2);
        if (piece > 0) mWhiteKingSquare = kingSquare;
        else mBlackKingSquare = kingSquare;
    } else {
        kingSquare = (piece > 0) ? mWhiteKingSquare : mBlackKingSquare;
    }

    // Check if king is in check
    bool inCheck = IsSquareAttacked(kingSquare, piece < 0);

    // Restore state
    RestoreState(savedState);

    return !inCheck;
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

bool Board::CheckBounds(int file, int rank) {
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

void Board::GeneratePawnMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    bool isWhite = pieceNum > 0;
    bool isPinned = IsPinned(file, rank);
    auto& moves = response ? mResponses : mPossibleMoves;
    int direction = isWhite ? -1 : 1;
    int startRank = isWhite ? 6 : 1;

    // Forward moves
    int newRank = rank + direction;
    if (CheckBounds(file, newRank) && mBoard[newRank][file] == 0) {
        if (!isPinned || IsPinnedDirectionValid(file, rank, isWhite, 0, direction)) {
            if (IsMoveLegal(pieceNum, file, rank, file, newRank)) {
                AddMove(currentSquare, file, newRank, moves);

                // Double push
                if (rank == startRank && mBoard[rank + 2*direction][file] == 0) {
                    if (IsMoveLegal(pieceNum, file, rank, file, rank + 2*direction)) {
                        AddMove(currentSquare, file, rank + 2*direction, moves);
                    }
                }
            }
        }
    }

    // Captures
    for (int i = -1; i <= 1; i += 2) {
        int newFile = file + i;
        if (CheckBounds(newFile, newRank)) {
            // Normal capture
            int target = mBoard[newRank][newFile];
            if (target != 0 && (target > 0) != isWhite) {
                if (!isPinned || IsPinnedDirectionValid(file, rank, isWhite, i, direction)) {
                    if (IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                        AddMove(currentSquare, newFile, newRank, moves);
                    }
                }
            }
            // En passant
            else if (!mEnPassantSquare.empty() && newFile == (mEnPassantSquare[0]-'a') &&
                    newRank == (8-(mEnPassantSquare[1]-'0'))) {
                if (!isPinned || IsPinnedDirectionValid(file, rank, isWhite, i, direction)) {
                    if (IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                        AddMove(currentSquare, newFile, newRank, moves);
                    }
                }
            }
        }
    }
}

void Board::GenerateKnightMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    if (IsPinned(file, rank)) return; // Knights can't move if pinned

    static const int knightMoves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    auto& moves = response ? mResponses : mPossibleMoves;
    bool isWhite = pieceNum > 0;

    for (const auto& move : knightMoves) {
        int newFile = file + move[0];
        int newRank = rank + move[1];
        if (CheckBounds(newFile, newRank)) {
            int target = mBoard[newRank][newFile];
            if (target == 0 || (target > 0) != isWhite) {
                if (IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                    AddMove(currentSquare, newFile, newRank, moves);
                }
            }
        }
    }
}

void Board::GenerateSlidingMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    bool isWhite = pieceNum > 0;
    bool isPinned = IsPinned(file, rank);
    auto& moves = response ? mResponses : mPossibleMoves;
    static const int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    for (const auto& dir : rookDirs) {
        if (isPinned && !IsPinnedDirectionValid(file, rank, isWhite, dir[0], dir[1])) continue;

        for (int dist = 1; dist < 8; dist++) {
            int newFile = file + dir[0]*dist;
            int newRank = rank + dir[1]*dist;
            if (!CheckBounds(newFile, newRank)) break;

            int target = mBoard[newRank][newFile];
            if (target == 0) {
                if (IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                    AddMove(currentSquare, newFile, newRank, moves);
                }
            } else {
                if ((target > 0) != isWhite && IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                    AddMove(currentSquare, newFile, newRank, moves);
                }
                break;
            }
        }
    }
}

void Board::GenerateDiagonalMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    bool isWhite = pieceNum > 0;
    bool isPinned = IsPinned(file, rank);
    auto& moves = response ? mResponses : mPossibleMoves;
    static const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};

    for (const auto& dir : bishopDirs) {
        if (isPinned && !IsPinnedDirectionValid(file, rank, isWhite, dir[0], dir[1])) continue;

        for (int dist = 1; dist < 8; dist++) {
            int newFile = file + dir[0]*dist;
            int newRank = rank + dir[1]*dist;
            if (!CheckBounds(newFile, newRank)) break;

            int target = mBoard[newRank][newFile];
            if (target == 0) {
                if (IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                    AddMove(currentSquare, newFile, newRank, moves);
                }
            } else {
                if ((target > 0) != isWhite && IsMoveLegal(pieceNum, file, rank, newFile, newRank)) {
                    AddMove(currentSquare, newFile, newRank, moves);
                }
                break;
            }
        }
    }
}

void Board::GenerateKingMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response) {
    bool isWhite = pieceNum > 0;
    auto& moves = response ? mResponses : mPossibleMoves;
    static const int kingMoves[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

    // Normal king moves
    for (const auto& move : kingMoves) {
        int newFile = file + move[0];
        int newRank = rank + move[1];
        if (CheckBounds(newFile, newRank)) {
            int target = mBoard[newRank][newFile];
            if (target == 0 || (target > 0) != isWhite) {
                // Temporarily move king to check safety
                BoardState saved = SaveState();
                mBoard[rank][file] = 0;
                mBoard[newRank][newFile] = pieceNum;
                std::string newKingSquare = std::string(1,'a'+newFile) + std::to_string(8-newRank);

                if (!IsSquareAttacked(newKingSquare, !isWhite)) {
                    AddMove(currentSquare, newFile, newRank, moves);
                }
                RestoreState(saved);
            }
        }
    }

    // Castling - only for non-response moves
    if (!response && !IsPinned(file, rank)) {
        bool kingsideAllowed = isWhite ? mWhiteCastlingKingsideRights : mBlackCastlingKingsideRights;
        bool queensideAllowed = isWhite ? mWhiteCastlingQueensideRights : mBlackCastlingQueensideRights;
        int castlingRank = isWhite ? 7 : 0;

        // Kingside castling
        if (kingsideAllowed) {
            bool pathClear = true;
            // Check squares between king and rook (f1,g1 for white)
            for (int f = file+1; f <= 6; f++) {
                if (mBoard[castlingRank][f] != 0) {
                    pathClear = false;
                    break;
                }
            }

            if (pathClear) {
                bool safePath = true;
                // Check if squares are under attack (e1,f1,g1 for white)
                for (int f = file; f <= file+2; f++) {
                    std::string square = std::string(1,'a'+f) + std::to_string(8-castlingRank);
                    if (IsSquareAttacked(square, !isWhite)) {
                        safePath = false;
                        break;
                    }
                }

                if (safePath) {
                    moves.push_back(isWhite ? "e1g1" : "e8g8");
                }
            }
        }

        // Queenside castling
        if (queensideAllowed) {
            bool pathClear = true;
            // Check squares between king and rook (b1,c1,d1 for white)
            for (int f = file-1; f >= 1; f--) {
                if (mBoard[castlingRank][f] != 0) {
                    pathClear = false;
                    break;
                }
            }

            if (pathClear && mBoard[castlingRank][0] == (isWhite ? 4 : -4)) { // Rook still there
                bool safePath = true;
                // Check if squares are under attack (e1,d1,c1 for white)
                for (int f = file; f >= file-2; f--) {
                    std::string square = std::string(1,'a'+f) + std::to_string(8-castlingRank);
                    if (IsSquareAttacked(square, !isWhite)) {
                        safePath = false;
                        break;
                    }
                }

                if (safePath) {
                    moves.push_back(isWhite ? "e1c1" : "e8c8");
                }
            }
        }
    }
}

// Helper function implementations
bool Board::IsMoveLegal(int pieceNum, int fromFile, int fromRank, int toFile, int toRank) {
    BoardState saved = SaveState();
    int captured = mBoard[toRank][toFile];
    mBoard[fromRank][fromFile] = 0;
    mBoard[toRank][toFile] = pieceNum;

    std::string kingSquare = (pieceNum > 0) ? mWhiteKingSquare : mBlackKingSquare;
    if (abs(pieceNum) == 6) { // Moving king
        kingSquare = std::string(1,'a'+toFile) + std::to_string(8-toRank);
    }

    bool legal = !IsSquareAttacked(kingSquare, pieceNum < 0);
    RestoreState(saved);
    return legal;
}

bool Board::IsPinnedDirectionValid(int file, int rank, bool isWhite, int dx, int dy) {
    std::string kingSquare = isWhite ? mWhiteKingSquare : mBlackKingSquare;
    int kingFile = kingSquare[0]-'a';
    int kingRank = 8-(kingSquare[1]-'0');

    // Check if movement direction aligns with king
    int fileDiff = file - kingFile;
    int rankDiff = rank - kingRank;

    // Same file/rank/diagonal check
    if (fileDiff == 0) return dx == 0; // Vertical movement only
    if (rankDiff == 0) return dy == 0; // Horizontal movement only
    if (abs(fileDiff) == abs(rankDiff)) return dx == (fileDiff > 0 ? 1 : -1) &&
                                             dy == (rankDiff > 0 ? 1 : -1);
    return false;
}

void Board::AddMove(const std::string& from, int toFile, int toRank, std::vector<std::string>& moves) {
    moves.push_back(from + std::string(1,'a'+toFile) + std::to_string(8-toRank));
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

        if (!mWhiteTurn) {
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