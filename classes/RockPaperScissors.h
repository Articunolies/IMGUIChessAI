#pragma once
#include "Game.h"
#include "Square.h"

//
// the classic game of Rock, Paper, Scissors
//
const int pieceSize = 100;

class RockPaperScissors : public Game
{
public:
    RockPaperScissors();
    ~RockPaperScissors();

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
    BitHolder &getHolderAt(const int x, const int y) override { return _grid[x]; }

private:
    Bit *PieceForPlayer(const int playerNumber);
    Player* ownerAt(int index );
    void    scanForMouse();
    Square   _grid[2];
};

