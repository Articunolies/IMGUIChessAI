#include "RockPaperScissors.h"

RockPaperScissors::RockPaperScissors()
{
}

RockPaperScissors::~RockPaperScissors()
{
}

//
// make a rock, paper, or scissors piece for the player
//
Bit* RockPaperScissors::PieceForPlayer(const int playerNumber)
{
    const char *textures[] = { "rps_rock.png", "rps_paper.png", "rps_scissors.png" };
    int random = rand() % 3;

    Bit *bit = new Bit();
    // should possibly be cached from player class?
    bit->LoadTextureFromFile(textures[random]);
    bit->setOwner(getCurrentPlayer());
    bit->setGameTag(random+1);
    bit->setSize(pieceSize, pieceSize);
    return bit;
}

void RockPaperScissors::setUpBoard()
{
    srand((unsigned int)time(0));
    setNumberOfPlayers(2);
    // this allows us to draw the board correctly
    _gameOptions.rowX = 2;
    _gameOptions.rowY = 1;
    // setup the board
    for (int x=0; x<2; x++) {
        _grid[x].initHolder(ImVec2(100*(float)x + 100, 100 + 100), "square.png", x, 0);
    }
    // if we have an AI set it up
    if (gameHasAI())
    {
        setAIPlayer(_gameOptions.AIPlayer);
    }
    // setup up turns etc.
    startGame();
}

//
// about the only thing we need to actually fill out for rock paper scissors
//
bool RockPaperScissors::actionForEmptyHolder(BitHolder& holder)
{
    if (holder.bit()) {
        return false;
    }
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());
    if (bit) {
        bit->setPosition(holder.getPosition());
        holder.setBit(bit);
        endTurn();
        return true;
    }   
    return false;
}

bool RockPaperScissors::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    // you can't move anything in rock paper scissors
    return false;
}

bool RockPaperScissors::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // you can't move anything in rock paper scissors
    return false;
}

//
// free all the memory used by the game on the heap
//
void RockPaperScissors::stopGame()
{
    for (int x=0; x<2; x++) {
        _grid[x].destroyBit();
    }
}

//
// helper function for the winner check
//
Player* RockPaperScissors::ownerAt(int index )
{
    if (index < 0 || index > 1) {
        return nullptr;
    }
    if (!_grid[index].bit()) {
        return nullptr;
    }
    return _grid[index].bit()->getOwner();
}

Player* RockPaperScissors::checkForWinner()
{
    int rps0 = _grid[0].bit() ? _grid[0].bit()->gameTag() : 0;
    int rps1 = _grid[1].bit() ? _grid[1].bit()->gameTag() : 0;
    // rock = 1, paper = 2, scissors = 3
    if (rps0 == 0 || rps1 == 0 || rps0 == rps1) {
        return nullptr;
    } else if ((rps0 - rps1 + 3) % 3 == 1) {
        return _players[0];
    } else {
        return _players[1];
    }    
    return nullptr;
}

bool RockPaperScissors::checkForDraw()
{
    int rps0 = _grid[0].bit() ? _grid[0].bit()->gameTag() : 0;
    int rps1 = _grid[1].bit() ? _grid[1].bit()->gameTag() : 0;
    // rock = 1, paper = 2, scissors = 3
    if ((rps0 != 0 && rps1 != 0) && rps0 == rps1) {
        return true;
    }
    return false;
}

//
// state strings
//
std::string RockPaperScissors::initialStateString()
{
    return "00";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string RockPaperScissors::stateString()
{
    std::string s;
    for (int x=0; x<2; x++) {
        Bit *bit = _grid[x].bit();
        if (bit) {
            s += std::to_string(bit->gameTag());
        } else {
            s += "0";
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void RockPaperScissors::setStateString(const std::string &s)
{
    for (int x=0; x<2; x++) {
        _grid[x].setBit( PieceForPlayer(0) );
    }
}

