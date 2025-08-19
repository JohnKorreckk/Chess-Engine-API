#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class Engine;

class Board {
public:
    Board(std::string& name, std::string& position);

    // Core game functions
    void MakeMove(const std::string& move);
    bool IsPinned(int file, int rank);
    bool CanCastle(bool kingside, bool white);
    // bool IsDraw();
    void UndoMove();
    bool IsLegalMove(const std::string& move);
    int CountMoves(int depth);

    // Move generation
    void GeneratePossibleMoves(bool response);
    std::vector<std::string> GetPossibleMoves() const { return mPossibleMoves; }

    // Board state
    std::vector<std::vector<int>> FenParser(std::string& fenString);
    std::string GenerateFen();
    void UpdateCheckStatus();
    bool IsSquareAttacked(const std::string& square, bool byWhite);
    std::vector<std::vector<int>> &GetBoard() { return mBoard; }

    // Utility functions
    void PrintInternalBoard();
    void Play();
    void PlayHuman();
    void PlayEngine();
    void displayWinner();

    // Getters
    bool IsWhiteTurn() const { return mWhiteTurn; }
    bool IsWhiteInCheck() const { return mWhiteInCheck; }
    bool IsBlackInCheck() const { return mBlackInCheck; }

private:
    struct BoardState {
        std::vector<std::vector<int>> board;
        std::string whiteKingSquare;
        std::string blackKingSquare;
        bool whiteTurn;
        bool whiteCastlingKingside;
        bool whiteCastlingQueenside;
        bool blackCastlingKingside;
        bool blackCastlingQueenside;
        bool whiteInCheck;
        bool blackInCheck;
        std::string enPassant;
    };

    struct MoveHistory {
        std::string move;
        int capturedPiece;
        bool whiteCastlingKingside;
        bool whiteCastlingQueenside;
        bool blackCastlingKingside;
        bool blackCastlingQueenside;
        std::string enPassantSquare;
        std::string whiteKingSquare;
        std::string blackKingSquare;
        int halfMoveClock;
        int fullMoveNumber;
    };

    // Board state
    std::vector<std::vector<int>> mBoard = std::vector<std::vector<int>>(8, std::vector<int>(8, 0));
    std::string mChessPosition;
    std::string mWhiteKingSquare;
    std::string mBlackKingSquare;
    bool mWhiteTurn;
    bool mWhiteInCheck;
    bool mBlackInCheck;
    int mHalfMoveClock = 0;
    int mFullMoveNumber = 0;
    std::unordered_map<std::string, int> mPositionHistory;

    // Castling rights
    bool mWhiteCastlingKingsideRights;
    bool mWhiteCastlingQueensideRights;
    bool mBlackCastlingKingsideRights;
    bool mBlackCastlingQueensideRights;

    // Move data
    std::string mEnPassantSquare;
    std::vector<std::string> mPossibleMoves;
    std::vector<std::string> mResponses;
    std::vector<MoveHistory> mHistory;

    // Engine
    std::shared_ptr<Engine> mEngine;

    // Private methods
    BoardState SaveState();
    void RestoreState(const BoardState& state);
    bool CheckBounds(int file, int rank);
    std::string PieceToString(int pieceNum);

    // Move generation helpers
    void GeneratePawnMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response);
    void GenerateKnightMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response);
    void GenerateSlidingMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response);
    void GenerateDiagonalMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response);
    void GenerateKingMoves(int pieceNum, int file, int rank, std::string currentSquare, bool response);
    bool IsMoveLegal(int pieceNum, int fromFile, int fromRank, int toFile, int toRank);
    void AddMove(const std::string& from, int toFile, int toRank, std::vector<std::string>& moves);
    bool IsPinnedDirectionValid(int file, int rank, bool isWhite, int dx, int dy);
};

#endif // BOARD_H