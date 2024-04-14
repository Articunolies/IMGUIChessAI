#include "TicTacToe.h"

const int AIPlayer = 1;
const int HumanPlayer = 0;

TicTacToe::TicTacToe()
{
}

TicTacToe::~TicTacToe()
{
}

//
// make a rock, paper, or scissors piece for the player
//
Bit* TicTacToe::PieceForPlayer(const int playerNumber)
{
    const char *textures[] = { "o.png", "x.png"};
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    bit->LoadTextureFromFile(textures[playerNumber]);
    bit->setOwner(getCurrentPlayer());
    bit->setGameTag(playerNumber+1);
    bit->setSize(pieceSize, pieceSize);
    return bit;

}

void TicTacToe::setUpBoard()
{
    setNumberOfPlayers(2);
    // this allows us to draw the board correctly
    // setup the board
    _gameOptions.rowX = 3;
    _gameOptions.rowY = 3;
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            _grid[y][x].initHolder(ImVec2(100*(float)x + 100, 100*(float)y + 100), "square.png", x, y);
        }
    }
    // if we have an AI set it up
    if (gameHasAI())
    {
        setAIPlayer(1);
    }
    // setup up turns etc.
    startGame();
}

void TicTacToe::scanForMouse()
{
	if (gameHasAI() && getCurrentPlayer()->isAIPlayer())
	{
		return;
	}
#if defined(UCI_INTERFACE)
	return;
#endif
	ImVec2 mousePos = ImGui::GetMousePos();
	mousePos.x -= ImGui::GetWindowPos().x;
	mousePos.y -= ImGui::GetWindowPos().y;

	Entity *entity = nullptr;
	for (int y = 0; y < _gameOptions.rowY; y++)
	{
		for (int x = 0; x < _gameOptions.rowX; x++)
		{
			BitHolder &holder = getHolderAt(x, y);
			Bit *bit = holder.bit();
			if (bit && bit->isMouseOver(mousePos))
			{
				entity = bit;
			}
			else if (holder.isMouseOver(mousePos))
			{
				entity = &holder;
			}
		}
	}
	if (ImGui::IsMouseClicked(0))
	{
		mouseDown(mousePos, entity);
	}
	else if (ImGui::IsMouseReleased(0))
	{
		mouseUp(mousePos, entity);
	}
	else
	{
		mouseMoved(mousePos, entity);
	}
}

//draw the board

void TicTacToe::drawFrame()
{
    scanForMouse();
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            _grid[y][x].paintSprite();
            if (_grid[y][x].bit()) {
                _grid[y][x].bit()->paintSprite();
            }
        }
    }
}
//
// about the only thing we need to actually fill out for TicTacToe
//
bool TicTacToe::actionForEmptyHolder(BitHolder& holder)
{
    if (holder.bit()) {
        return false;
    }
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());
    if (bit) {
        bit->setPosition(holder.getPosition());
        holder.setBit(bit);
        if(getCurrentPlayer()->playerNumber() == 0){
            endTurn();
        }
        return true;
    }   
    return false;
}

bool TicTacToe::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    // you can't move anything in TicTacToe
    return false;
}

bool TicTacToe::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // you can't move anything in TicTacToe
    return false;
}

//
// free all the memory used by the game on the heap
//
void TicTacToe::stopGame()
{
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            _grid[y][x].destroyBit();
        }
    }
}

//
// helper function for the winner check
//
Player* TicTacToe::ownerAt(int index ) const
{
    if(!_grid[index/3][index%3].bit()) {
        return nullptr;
    }
    return _grid[index/3][index%3].bit()->getOwner();
}

Player* TicTacToe::checkForWinner()
{
    // check for a winner
    // 0 1 2
    // 3 4 5
    // 6 7 8
    // horizontal
    static const int WinningBoards[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
        {0, 4, 8}, {2, 4, 6}
    };

    for (int i=0; i<8; i++) {
        const int *board = WinningBoards[i];
        Player *player = ownerAt(board[0]);
        if (player && player == ownerAt(board[1]) && player == ownerAt(board[2])) {
            return player;
        }
    }
    return nullptr;
}

bool TicTacToe::checkForDraw()
{
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            if (!_grid[y][x].bit()) {
                return false;
            }
        }
    }
    return true;
}

//
// state strings
//
std::string TicTacToe::initialStateString()
{
    return "000000000";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string TicTacToe::stateString()
{
    std::string s;
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            if (_grid[y][x].bit()) {
                s += std::to_string(_grid[y][x].bit()->gameTag());
            } else {
                s += "0";
            }
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void TicTacToe::setStateString(const std::string &s)
{
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            int index = y*3+x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber-1));
            } else{
                _grid[y][x].setBit(nullptr);
            }

        }
    }
}



void TicTacToe::updateAI()
{
    int bestVal = -1000;
    Square *bestMove = nullptr;

    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            if (!_grid[y][x].bit()) {
                _grid[y][x].setBit(PieceForPlayer(1));
                TicTacToeAI *newGame = this->clone();
                int moveVal = newGame->negamax(newGame, 0, 1);
                delete newGame;
                _grid[y][x].setBit(nullptr);
                if (moveVal > bestVal) {
                    bestMove = &_grid[y][x];
                    bestVal = moveVal;
                }
            }
        }
    }
    if (bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
            endTurn();
        }
    }
}

TicTacToeAI* TicTacToe::clone()
{
    TicTacToeAI* newGame = new TicTacToeAI();
    std::string gamestate = stateString();
    for(int y=0; y<3; y++) {
        for(int x=0; x<3; x++) {
            int index = y*3+x;
            int playerNumber = gamestate[index] - '0';
            newGame->_grid[y][x] = playerNumber;
        }
    }
    return newGame;

}

int TicTacToeAI::ownerAt(int index ) const
{
    return _grid[index/3][index%3];
}

int TicTacToeAI::AICheckForWinner()
{
    // check for a winner
    // 0 1 2
    // 3 4 5
    // 6 7 8
    // horizontal
    static const int WinningBoards[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
        {0, 4, 8}, {2, 4, 6}
    };

    for (int i=0; i<8; i++) {
        const int *board = WinningBoards[i];
        int playerInt = ownerAt(board[0]);
        if (playerInt && playerInt == ownerAt(board[1]) && playerInt == ownerAt(board[2])) {
            return playerInt;
        }
    }
    return -1;
}

int TicTacToeAI::evaluateBoard()
{
    int winner = AICheckForWinner();
    if (winner == 0) {
        return 0;
    } else if (winner == HumanPlayer-1) {
        return -10;
    }
    return 10;
}

bool TicTacToeAI::isBoardFull() const
{
    for (int y=0; y<3; y++) {
        for(int x=0; x<3; x++)
        {
            if (_grid[y][x] == 0) {
                return false;
            }
        }
    }
    return true;
}

int TicTacToeAI::negamax(TicTacToeAI* state, int depth, int playercolor){
    int score = state->evaluateBoard();

    if(score == 10){
        return playercolor * (score - depth);
    }
    if (state->isBoardFull()) {
        return 0;
    }
    int bestVal = -1000;
    for(int y=0; y<3; y++) {
        for(int x=0; x<3; x++) {
            if (!state->_grid[y][x]) {
                state->_grid[y][x] = (playercolor == -1) ? 1 : 2;
                bestVal = std::max(bestVal, -negamax(state, depth+1, -playercolor));
                state->_grid[y][x] = 0;
            }
        }
    }
    return bestVal;
}
