#pragma once
#include "Game.h"
#include "Square.h"

//
// the classic game of TicTacToe
//


class TicTacToeAI
{
public:
    int _grid[3][3];
    bool isBoardFull() const;
    int evaluateBoard();
    int negamax(TicTacToeAI* state, int depth, int playercolor);
    int ownerAt(int index ) const;
    int AICheckForWinner();
};

class TicTacToe : public Game
{
public:
    TicTacToe();
    ~TicTacToe();

    // set up the board
    void setUpBoard() override;

    Player* checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    bool canBitMoveFrom(Bit&bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void stopGame() override;
    void drawFrame() override;
    BitHolder &getHolderAt(const int x, const int y) override { return _grid[y][x]; }
    TicTacToeAI* clone();
    void updateAI() override;
    bool gameHasAI() override {return true;}


private:
    Bit *PieceForPlayer(const int playerNumber);
    Player* ownerAt(int index ) const;
    void    scanForMouse();
    Square   _grid[3][3];
};

