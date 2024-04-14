#pragma once
#include "Game.h"
#include "ChessSquare.h"
#include "Evaluate.h"



//
// the classic game of Chess
//
const int pieceSize = 64;


enum ChessPiece
{
    Pawn = 1,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

enum CastleStatus{
    WhiteKingMoved = 1,
    WhiteRookAMoved = 2,
    WhiteRookHMoved = 4,
    BlackKingMoved = 8,
    BlackRookAMoved = 16,
    BlackRookHMoved = 32
};

class Chess : public Game
{
public:
    Chess();
    ~Chess();
    struct Move {
        std::string from;
        std::string to;
    };
    // set up the board
    void setUpBoard() override;
    Player* checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    bool actionForEmptyHolder(BitHolder &holder) override {return false;};
    bool canBitMoveFrom(Bit&bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void stopGame() override;

    void updateAI() override;
    bool gameHasAI() override {return true; }

    BitHolder &getHolderAt(const int x, const int y) override { return _grid[y][x]; }

    void FENtoBoard(const std::string& fen);
    void        UCIMove(const std::string& move);


private:
    Bit *PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int index) const;
    
    void addMoveIfValid(std::vector<Move>& moves, int fromRow, int fromCol, int toRow, int toCol);
    std::string indexToNotation(int row, int col);
    std::string pieceNoation(int row, int column) const;
    void generateKnightMoves(std::vector<Move>& moves, int row, int col);
    void generatePawnMoves(std::vector<Move>& moves, int row, int col, std::string lastMove, char color);
    void generateLinearMoves(std::vector<Move>& moves, int row, int col, const std::vector<std::pair<int, int>>& directions);
    void generateRookMoves(std::vector<Move>& moves, int row, int col);
    void generateBishopMoves(std::vector<Move>& moves, int row, int col);
    void generateKingMoves(std::vector<Move>& moves, int row, int col, char color);
    void generateQueenMoves(std::vector<Move>& moves, int row, int col);
    char oppositeColor(char color);
    int notationToIndex(std::string &notation);
    int evaluateBoard(const char* state);
    char stateNotation(const char* state, int row, int col);
    int stateColor(const char* state, int row, int col);
    int negamax(char* state, int depth, int alpha, int beta, int playerColor);
    std::vector<Chess::Move> generateMoves(std::string lastMove, char color, bool filter);
    void filterOutIllegalMoves(std::vector<Chess::Move>& moves, char color);
 
    ChessSquare   _grid[8][8];
    std::vector<Chess::Move> _moves;
    int _castleStatus;
    int _countSearch;
    int _winner = 0;
    std::string _lastMove;
};

