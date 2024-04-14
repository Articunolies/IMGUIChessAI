#include "Chess.h"
#include "UCI.h"
#include "Evaluate.h"

#if defined(UCI_INTERFACE)
static UCIInterface uciInterface;
#endif

std::string Chess::pieceNoation(int row, int column) const
{
    const char *wpieces = "?PNBRQK";
    const char *bpieces = "?pnbrqk";
    std::string notation = "";
    Bit *bit = _grid[row][column].bit();
    if (bit) {
        notation += bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()&0x7f];
    } else {
        notation = "0";
    }
    return notation;
}

Chess::Chess()
{
}

Chess::~Chess()
{
}

//
// make a rock, paper, or scissors piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char *pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png"};
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    const char *pieceName = pieces[piece-1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);
    return bit;

}



void Chess::setUpBoard()
{
    const ChessPiece initialBoard[2][8] = {
        {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook},
        {Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn}
    };

    
    setNumberOfPlayers(2);
    // this allows us to draw the board correctly
    // setup the board
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    for (int y=0; y<8; y++) {
        for(int x=0; x<8; x++)
        {
            ImVec2 position((float)(pieceSize*x+pieceSize), (float)(pieceSize*(_gameOptions.rowY - y)+ pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            _grid[y][x].setNotation(indexToNotation(y, x));
        }
    }

    // for (int rank=0; rank<2; rank++) {
    //     for(int x = 0; x<8; x++){
    //         ChessPiece piece = initialBoard[rank][x];
    //         Bit *bit = PieceForPlayer(0, initialBoard[rank][x]); //White pieces = 0
    //         bit->setPosition(_grid[rank][x].getPosition());
    //         bit->setParent(&_grid[rank][x]);
    //         bit->setGameTag(piece+128);
    //         _grid[rank][x].setBit(bit);

    //         bit = PieceForPlayer(1, initialBoard[rank][x]);     //Black piece = 1
    //         bit->setPosition(_grid[7-rank][x].getPosition());
    //         bit->setParent(&_grid[7-rank][x]);
    //         bit->setGameTag(piece);
    //         _grid[7-rank][x].setBit(bit);

    //     }
    
    // }     
     
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    //FENtoBoard("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1");
    // if we have an AI set it up
    if (gameHasAI())
    {
        setAIPlayer(1);
    }
    _moves = generateMoves(_lastMove, 'w', true);
    // setup up turns etc.
    
    startGame();
    std::cout << stateString() << std::endl;
#if defined(UCI_INTERFACE)
uciInterface.Run(this);
#endif   

}


void Chess::FENtoBoard(const std::string& fen)
{
    //clear the board
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            _grid[i][j].setBit(nullptr);
        }
    }
    
    std::istringstream femnStream(fen);
    std::string boardPart;
    std::getline(femnStream, boardPart, ' ');
    int row = 7;
    int col = 0;
    for (char c : boardPart)
    {
        if (c == '/')
        {
            row--;
            col = 0;
        }
        else if (isdigit(c))
        {
            col += c - '0';
        }
        else
        {
            ChessPiece piece = Pawn;
            switch (toupper(c))
            {
            case 'P':
                piece = Pawn;
                break;
            case 'N':
                piece = Knight;
                break;
            case 'B':
                piece = Bishop;
                break;
            case 'R':
                piece = Rook;
                break;
            case 'Q':
                piece = Queen;
                break;
            case 'K':
                piece = King;
                break;
            }
            Bit *bit = PieceForPlayer(isupper(c) ? 0 : 1, piece);
            bit->setPosition(_grid[row][col].getPosition());
            bit->setParent(&_grid[row][col]);
            bit->setGameTag(isupper(c) ? piece : (piece + 128));
            bit->setSize(pieceSize, pieceSize);
            _grid[row][col].setBit(bit);
            col++;
        }
    }
}


bool Chess::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    for(auto move : _moves){
        if(move.from == srcSquare.getNotation()){
            return true;
        }
    }
    return false;
}
bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // you can't move anything in Chess
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);
    for(auto move : _moves){
        if(move.from == srcSquare.getNotation() && move.to == dstSquare.getNotation()){
            return true;
        }
    }
    return false;

}

void Chess::bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    const char *bpieces = "pnbrqk";
    const char *wpieces = "PNBRQK";
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);
    _lastMove = "x-"+ srcSquare.getNotation() + "-" + dstSquare.getNotation();
    _lastMove[0] = (bit.gameTag() < 128) ? wpieces[bit.gameTag()- 1] : bpieces[bit.gameTag()- 129];
    
    //pawn promotion
    if (((bit.gameTag() & 0x7f) == Pawn) && ((dstSquare.getRow() == 0) || dstSquare.getRow() == 7)) {
        int playerNumber = (bit.gameTag() & 0x80) ? 0 : 1;
        Bit* newBit = PieceForPlayer(playerNumber, Queen);
        newBit->setPosition(dstSquare.getPosition());
        newBit->setParent(&dstSquare);
        newBit->setGameTag((bit.gameTag() < 128) ? Queen : (Queen + 128));
        dstSquare.setBit(newBit);
    }
    //en passant

    if (((bit.gameTag() & 0x7f) == Pawn) && (srcSquare.getColumn() != dstSquare.getColumn())) {
        int row = (bit.gameTag() < 128) ? 4 : 3;
        BitHolder &enPassantSquare = getHolderAt(dstSquare.getColumn(), row);
        Bit *enPassantPiece = enPassantSquare.bit();
        if (enPassantPiece && enPassantPiece->gameTag() != bit.gameTag()) {
            enPassantSquare.setBit(nullptr);
        }
    }

    //castling
    switch (_lastMove[0]){
        case 'K':
            _castleStatus |= WhiteKingMoved;
            break;
        case 'k':
            _castleStatus |= BlackKingMoved;
            break;
        case 'R':
            if (_lastMove[1] == 'a'){
                _castleStatus |= WhiteRookAMoved;
            } else if (_lastMove[1] == 'h'){
                _castleStatus |= WhiteRookHMoved;
            }
            break;
        case 'r':
            if (_lastMove[1] == 'a'){
                _castleStatus |= BlackRookAMoved;
            } else if (_lastMove[1] == 'h'){
                _castleStatus |= BlackRookHMoved;
            }
            break;
    }

    if (dstSquare.getNotation() == "a1") _castleStatus |= WhiteRookAMoved;
    if (dstSquare.getNotation() == "h1") _castleStatus |= WhiteRookHMoved;
    if (dstSquare.getNotation() == "a8") _castleStatus |= BlackRookAMoved;
    if (dstSquare.getNotation() == "h8") _castleStatus |= BlackRookHMoved;
    

    int distance = srcSquare.getDistance(dstSquare);
    if (((bit.gameTag() & 127) == ChessPiece::King) && (distance == 2)) {
        int rookSrcCol = (dstSquare.getColumn() == 6) ? 7 : 0;
        int rookDstCol = (dstSquare.getColumn() == 6) ? 5 : 3;
        BitHolder& rookSrc = getHolderAt(rookSrcCol, dstSquare.getRow());
        BitHolder& rookDst = getHolderAt(rookDstCol, dstSquare.getRow());
        Bit* rook = rookSrc.bit();
        if (rook) {
            rookDst.setBit(rook);
            rookSrc.setBit(nullptr);
            rook->setPosition(rookDst.getPosition());
        }
    }

    Game::bitMovedFromTo(bit, src, dst);
    _moves = generateMoves(_lastMove, (_gameOptions.currentTurnNo&1) ? 'b' : 'w', true);
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
    for (int y=0; y<8; y++) {
        for(int x=0; x<8; x++)
        {
            _grid[y][x].destroyBit();
        }
    }
}



Player* Chess::checkForWinner()
{
    if (_winner == 1){
        return getPlayerAt(_gameOptions.currentTurnNo&1 ? 0 : 1);
    }
    return nullptr;

}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y=0; y<8; y++) {
        for(int x=0; x<8; x++)
        {
            s +=pieceNoation(y, x);
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y=0; y<8; y++) {
        for(int x=0; x<8; x++)
        {
            int index = y*8+x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber-1, Pawn));
            } else{
                _grid[y][x].setBit(nullptr);
            }

        }
    }
}


void Chess::addMoveIfValid(std::vector<Move>& moves, int fromRow, int fromCol, int toRow, int toCol)
{
    if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8) {
        if(pieceNoation(fromRow,fromCol)[0] != pieceNoation(toRow, toCol)[0]){
            moves.push_back({indexToNotation(fromRow, fromCol), indexToNotation(toRow, toCol)});
        }
    }
}

std::string Chess::indexToNotation(int row, int col){
    return std::string(1, 'a'+col) + std::string(1, '1'+row);
}

void Chess::generateKnightMoves(std::vector<Move>& moves, int row, int col)
{
    int rowOffsets[] = {2, 1, -1, -2, -2, -1, 1, 2};
    int colOffsets[] = {1, 2, 2, 1, -1, -2, -2, -1};
    for (int i=0; i<8; i++) {
        addMoveIfValid(moves, row, col, row+rowOffsets[i], col+colOffsets[i]);
    }
}

void Chess::generatePawnMoves(std::vector<Move>& moves, int row, int col, std::string lastMove, char color)
{
    int direction = (color == 'w') ? 1 : -1;
    int startRow = (color == 'w') ? 1 : 6;

    if (pieceNoation(row+direction, col) == "0") {
        addMoveIfValid(moves, row, col, row+direction, col);
        if (row == startRow && pieceNoation(row+2*direction, col) == "0") {
            addMoveIfValid(moves, row, col, row+2*direction, col);
        }
    }

    for (int i = -1; i <= 1; i+=2) {
        if (col + i >= 0 && col + i < 8) {
            int pieceColor = (pieceNoation(row+direction, col+i)[0] < 'a') ? 1 : -1; 
            int colorAsInt = (color == 'w') ? 1 : -1;
            if (pieceColor != colorAsInt) {
                addMoveIfValid(moves, row, col, row+direction, col+i);
            }
        }
    }
    // for (int i = -1; i <= 1; i += 2) { // -1 for left, +1 for right
    //     if (col + i >= 0 && col + i < 8) {
    //         int oppositeColor = (colorAsInt == 0) ? 1 : -1;
    //         int pieceColor = stateColor( state, row + direction, col + i);
    //         if (pieceColor == oppositeColor) {
    //             addMoveIfValid(state, moves, row, col, row + direction, col + i);
    //         }
    //     }
    // }
    // Last move structure
    if (lastMove.length()){
        char lastMovePiece = lastMove[0];          //piece in 0
        int lastMoveStartRow = lastMove[3] - '0';
        int lastMoveEndRow = lastMove[6] - '0';
        int lastMoveStartCol = lastMove[2] - 'a';
        int lastMoveEndCol = lastMove[5] - 'a';
    
        if (color == 'w' && row == 4) {
            if (lastMovePiece == 'p' && lastMoveStartRow == 7 && lastMoveEndRow == 5) {
                if(lastMoveEndCol == col-1 || lastMoveEndCol == col+1)
                    addMoveIfValid(moves, row, col, row+1, lastMoveEndCol);
                
            }
        } else if (color == 'b' && row == 4) {
            if (lastMovePiece == 'P' && lastMoveStartRow == 2 && lastMoveEndRow == 4) {
                if(lastMoveEndCol == col-1 || lastMoveEndCol == col+1)
                    addMoveIfValid(moves, row, col, row+1, lastMoveEndCol);
            }
        }
    }
}

void Chess::generateLinearMoves(std::vector<Move>& moves, int row, int col, const std::vector<std::pair<int, int>>& directions)
{
    for (auto& direction : directions) {
        int rowOffset = row + direction.first;
        int colOffset = col + direction.second;
        while(rowOffset >= 0 && rowOffset < 8 && colOffset >= 0 && colOffset < 8){
            if (pieceNoation(rowOffset, colOffset) != "0") {
                addMoveIfValid(moves, row, col, rowOffset, colOffset);
                break;
            }
        addMoveIfValid(moves, row, col, rowOffset, colOffset);
            rowOffset += direction.first;
            colOffset += direction.second;
        }    
    }
}

void Chess::generateBishopMoves(std::vector<Move>& moves, int row, int col)
{
    std::vector<std::pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    generateLinearMoves(moves, row, col, directions);
}

void Chess::generateRookMoves(std::vector<Move>& moves, int row, int col)
{
    std::vector<std::pair<int, int>> directions = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    generateLinearMoves(moves, row, col, directions);
}

void Chess::generateQueenMoves(std::vector<Move>& moves, int row, int col)
{
    generateRookMoves(moves, row, col);
    generateBishopMoves(moves, row, col);
}

void Chess::generateKingMoves(std::vector<Move>& moves, int row, int col, char color)
{
    std::vector<std::pair<int, int>> directions = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
    for( auto& direction : directions){
        int newRow = row + direction.first;
        int newCol = col + direction.second;
        if(newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8){
            addMoveIfValid(moves, row, col, newRow, newCol);
        }
    }
    //castling
    int colorAsInt = (color == 'w') ? 1 : -1;
    if(_castleStatus && (colorAsInt == 1)){
        if(pieceNoation(0, 5) == "0" && pieceNoation(0, 6) == "0"){
                addMoveIfValid(moves, 0, 4, 0, 6);
            }
        if(!(_castleStatus & WhiteRookAMoved)){
            if(pieceNoation(0, 1) == "0" && pieceNoation(0, 2) == "0" && pieceNoation(0, 3) == "0"){
                addMoveIfValid(moves, 0, 4, 0, 2);
            }
        }
    } else if (_castleStatus && (colorAsInt == -1)){
        if(!(_castleStatus & BlackRookHMoved)){
            if(pieceNoation(7, 5) == "0" && pieceNoation(7, 6) == "0"){
                addMoveIfValid(moves, 7, 4, 7, 6);
            }
        }
        if(!(_castleStatus & BlackRookAMoved)){
            if(pieceNoation(7, 1) == "0" && pieceNoation(7, 2) == "0" && pieceNoation(7, 3) == "0"){
                addMoveIfValid(moves, 7, 4, 7, 2);
            }
        }
    }


    
}

void Chess::filterOutIllegalMoves(std::vector<Chess::Move>& moves, char color){
    char baseState[65];
    std::string copyState = std::string(stateString().c_str());

    int kingSquare = -1;
    for (int i = 0; i < 64; i++){
        if (copyState[i] == 'k' && color == 'b'){ kingSquare = i; break;}
        if(copyState[i] == 'K' && color == 'w') {kingSquare = i; break;}
    }
    for (auto it = moves.begin(); it != moves.end();){
        bool moveBad = false; 
        memcpy(&baseState[0],copyState.c_str(), 64);
        int srcSquare = notationToIndex(it->from);
        int dstSquare = notationToIndex(it->to);
        baseState[dstSquare] = baseState[srcSquare];
        baseState[srcSquare] = '0';
        // Handle the case in which the king is the piece that moved, and it may have left or remained in check
        int updatedKingSquare = kingSquare;
        if (baseState[dstSquare] == 'k' && color == 'b')
        {
            updatedKingSquare = dstSquare;
        }
        else if (baseState[dstSquare] == 'K' && color == 'w')
        {
            updatedKingSquare = dstSquare;
        }
        auto oppositeMoves = generateMoves(baseState, color == 'w' ? 'b' : 'w', false);
        for (auto enemyMoves : oppositeMoves) {
            int enemyDst = notationToIndex(enemyMoves.to);
            if (enemyDst == updatedKingSquare){
                moveBad = true;
                break;
            }
        }
        if (moveBad){
            it = moves.erase(it);
        }
        else {
            ++it;
        }
        
        if (moves.size() <= 4) {
            std::cout << "Game Over" << std::endl;
            _winner = 1;
        }
    }
}

std::vector<Chess::Move> Chess::generateMoves(std::string lastMove, char color, bool filter)
{
    std::vector<Move> moves;
    for (int row=0; row<8; row++) {
        for(int col=0; col<8; col++)
        {
            std::string piece = pieceNoation(row, col);
            int pieceColor = (piece[0] == '0') ? 0 : (piece[0] < 'a') ? 1 : -1; 
            int colorAsInt = (color == 'w') ? 1 : -1;
            if (!piece.empty() && piece != "0" && pieceColor == colorAsInt) {
                switch (toupper(piece[0])) {
                    case 'P':
                        generatePawnMoves(moves, row, col, lastMove, color);
                        break;
                    case 'N':
                        generateKnightMoves(moves, row, col);
                        break;
                    case 'B':
                        generateBishopMoves(moves, row, col);
                        break;
                    case 'R':
                        generateRookMoves(moves, row, col);
                        break;
                    case 'Q':
                        generateQueenMoves(moves, row, col);
                        break;
                    case 'K':
                        generateKingMoves(moves, row, col, color);
                        break;
                }
            }
        }
    }
    // if(filter){
    //     filterOutIllegalMoves(moves, color);
    // }
    return moves;
}

int Chess::notationToIndex(std::string &notation)
{
    int square = notation[0] - 'a';
    square += (notation[1] - '1') * 8;
    return square;
}

void Chess::updateAI(){
    char baseState[65];
    const int myinfinity = 9999999; 
    int bestMoveScore = -myinfinity;
    Move bestMove;
    std::string copyState = stateString();
    for(auto move : _moves){
        _countSearch = 0;
        memcpy(&baseState[0], copyState.c_str(),64);
        int srcSquare = notationToIndex(move.from);
        int dstSquare = notationToIndex(move.to);
        baseState[dstSquare] = baseState[srcSquare];
        baseState[srcSquare] = '0';
        int score = -negamax(baseState, 3, -myinfinity, myinfinity, 1);
        if (score > bestMoveScore) {
            bestMoveScore = score;
            bestMove = move;
        }
    }
#if defined(UCI_INTERFACE)
    int srcSquare = notationToIndex(bestMove.from);
    int dstSquare = notationToIndex(bestMove.to);
    std::string bestUCIMove = indexToNotation(srcSquare/8, srcSquare%8);
    bestUCIMove += std::string(indexToNotation(dstSquare/8, dstSquare%8));
    uciInterface.UCILog("best move: " + bestUCIMove);
//    if (bestMove.flags & MoveFlags::IsPromotion) {
//        bestUCIMove += "q";
//    }
    uciInterface.SendMove(bestUCIMove);
#else
    std::cout << "searched " << _countSearch << " nodes" << std::endl;
#endif
    if (bestMoveScore != -9999999) {
        int srcSquare = notationToIndex(bestMove.from);
        int dstSquare = notationToIndex(bestMove.to);
        BitHolder& src = getHolderAt(srcSquare&7, srcSquare/8);
        BitHolder& dst = getHolderAt(dstSquare&7, dstSquare/8);
        Bit* bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        src.setBit(nullptr);
        bitMovedFromTo(*bit, src, dst);
        std::cout << "searched " << _countSearch << " nodes" << std::endl;
    }
}

int Chess::evaluateBoard(const char* state)
{
    std::map<char, int> scores = {
        {'P', 100}, {'p', -100},
        {'N', 200}, {'n', -200},
        {'B', 230}, {'b', -230},
        {'R', 400}, {'r', -400},
        {'Q', 900}, {'q', -900},
        {'K', 2000}, {'k', -2000},
        {'0', 0}
    };
    int score = 0;
    for(int i=0; i<64; i++) {
        score += scores[state[i]];
    }
    for(int i=0; i<64; i++) {
        char piece = state[i];
        switch (piece){
            case 'N':
                score += knightTable[FLIP(i)];
                break;
            case 'n':
                score -= knightTable[i];
                break;
            case 'P':
                score += pawnTable[FLIP(i)];
                break;
            case 'p':
                score -= pawnTable[i];
                break;
            case 'R':
                score += rookTable[FLIP(i)];
                break;
            case 'r':
                score -= rookTable[i];
                break;
            case 'Q':
                score += queenTable[FLIP(i)];
                break;
            case 'q':
                score -= queenTable[i];
                break;
        }
    }    
    return score;
}

int Chess::negamax(char* state, int depth, int alpha, int beta, int playerColor)
{
    _countSearch++;
    if (depth == 0) {
        int score = evaluateBoard(state);
        return playerColor * score;
    }
    int bestVal = -9999999;
    auto negaMoves = generateMoves(state, (playerColor == 1) ? 'w' : 'b', true);
    for(auto move : negaMoves){
        int srcSquare = notationToIndex(move.from);
        int dstSquare = notationToIndex(move.to);
        char saveMove = state[dstSquare];
        state[dstSquare] = state[srcSquare];
        state[srcSquare] = '0';
        bestVal = std::max(bestVal, -negamax(state, depth-1, -beta, -alpha, -playerColor));
        alpha = std::max(alpha, bestVal);
        state[srcSquare] = state[dstSquare];
        state[dstSquare] = saveMove;
        if (alpha >= beta) {
            break;
        }
    }
    return bestVal;
}

void Chess::UCIMove(const std::string& move) {
#if defined(UCI_INTERFACE)
    int fromCol = move[0] - 'a';
    int fromRow = move[1] - '1';
    int toCol = move[2] - 'a';
    int toRow = move[3] - '1';
    BitHolder& src = getHolderAt(fromCol, fromRow);
    BitHolder& dst = getHolderAt(toCol, toRow);
    Bit* bit = src.bit();
    if (bit) {
        if (dst.bit()) {
            pieceTaken(dst.bit());
        }
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        src.setBit(nullptr);
        // this also calls endTurn
        bitMovedFromTo(*bit, src, dst);
        uciInterface.UCILog("Chess::UCIMove: " + move);
    }
#endif
}
