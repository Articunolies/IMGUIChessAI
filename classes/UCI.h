#pragma once

/**
 * @class ChessEngineUCI
 * @brief Represents a UCI-compliant chess engine.
 *
 * This class provides an interface for a UCI-compliant chess engine
 * to communicate with a chess GUI or other UCI-compatible programs.
 */

#include "Chess.h"
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

const std::string UCI_CMD_ISREADY = "isready";
const std::string UCI_CMD_UCINEWGAME = "ucinewgame";
const std::string UCI_CMD_POSITION = "position";
const std::string UCI_CMD_GO = "go";
const std::string UCI_CMD_STOP = "stop";
const std::string UCI_CMD_QUIT = "quit";
const std::string UCI_CMD = "uci";
const std::string UCI_CMD_MOVE = "move";
const std::string UCI_CMD_MOVES = "moves";

class UCIInterface {
  public:
    UCIInterface()
        : _running(false)
        , _instance(nullptr) {
        _state = "";
        // Initialize any necessary variables or resources here.
    }

    // Function to start the UCI communication loop, this needs to be on a thread.
    void Run(Chess* instance) {
        if (_running) {
            return;
        }
        _running = true;
        _instance = instance;
        // Send the engine's identity and available options in UCI format.
        SendUCIInfo();
        UCILog("RUNNING NEW GAME");
        _state = "";
        // ProcessUCICommand("ucinewgame");
        // ProcessUCICommand("position startpos moves d2d4 g8f6 e2e4 f6e4 c2c4 d7d5");
        ReadInput();
    }

    // Function to read input from the console.
    void ReadInput() {
        std::string input;
        while (true) {
            std::getline(std::cin, input);
            // Process the received UCI command.
            ProcessUCICommand(input);
        }
    }

    void SendMove(std::string move) {
        std::cout << "bestmove " << move << std::endl;
        std::cout.flush();
        UCILog("bestmove " + std::string(move));
    }

    void SendUCIUpdate(const int depth, const int movespersecond, const int nodes, const int score, const std::string& pv) {
        std::string output = "info depth " + std::to_string(depth) + " nodes " + std::to_string(nodes) + " nps " + std::to_string(movespersecond) + " pv " + pv;
        std::cout << output << std::endl;
        std::cout.flush();
        UCILog(output);
    }

    void UCILog(std::string message);

  private:
    void ProcessMoves(std::istringstream& iss) {
        // look for moves
        std::string move;
        iss >> move;
        int player = 0;
        if (move == UCI_CMD_MOVES) {
            while (iss >> move) {
                UCILog("move : " + move);
                _instance->UCIMove(move);
                player++;
            }
            // make the game sync to the correct player
            int onPlayer = (player & 1);
            if (_instance->getCurrentPlayer()->playerNumber() != onPlayer) {
                _instance->endTurn();
            }
        } else {
            // make the game sync to the correct player
            int onPlayer = (player & 1);
            if (_instance->getCurrentPlayer()->playerNumber() != onPlayer) {
                _instance->endTurn();
            }
        }
    }
    // Function to parse and handle incoming UCI commands.
    void ProcessUCICommand(const std::string& command) {
        std::istringstream iss(command);
        std::string token;
        iss >> token;

        UCILog(command);

        if (token == UCI_CMD) {
            // Handle the "uci" command by sending engine info and options.
            SendUCIInfo();
        } else if (token == UCI_CMD_UCINEWGAME) {
            _instance->setUpBoard();
            _instance->FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
        } else if (token == UCI_CMD_ISREADY) {
            // Handle the "isready" command by indicating the engine is ready.
            std::cout << "readyok" << std::endl;
            std::cout.flush();
        } else if (token == UCI_CMD_POSITION) {
            // Handle the "position" command by setting up the board position.
            // Parse the rest of the command to set the position.
            // Example: "position startpos moves e2e4 e7e5"
            // Example: "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR moves e2e4 e7e5"
            std::string positionType;
            iss >> positionType;
            if (positionType == "startpos") {
                _instance->setUpBoard();
                UCILog("startpos :rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
                // Handle the "startpos" command by setting up the board position to the starting position.
                _instance->FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
                ProcessMoves(iss);
            } else if (positionType == "fen") {
                // Handle the "fen" command by setting up the board position to the given FEN string.
                // Parse the rest of the command to get the FEN string.
                // Example: "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
                std::string fen;
                iss >> fen;
                if (fen != _state) {
                    _instance->FENtoBoard(fen);
                    _state = fen;
                }
                ProcessMoves(iss);
            } else {
                UCILog("ERROR : " + positionType);
            }
        } else if (token == UCI_CMD_GO) {
            int playerNumber = _instance->getCurrentPlayer()->playerNumber();
            playerNumber == 0 ? UCILog("AI IS WHITE") : UCILog("AI IS BLACK");
            _instance->setAIPlayer(playerNumber);
            _instance->updateAI();
        } else if (token == UCI_CMD_STOP) {
            // Handle the "stop" command by stopping the engine's search.
            // Stop searching and send the best move found so far.
        } else if (token == UCI_CMD_MOVE) {
            // Handle the "move" command by processing the actual moves from the other AI.
            // Example: "move e2e4"
            std::string move;
            iss >> move;
            _instance->UCIMove(move);
            UCILog("move : " + move);
            // Process the move received from the other AI
            // Update your board state accordingly
        } else if (token == UCI_CMD_QUIT) {
            // Handle the "quit" command by gracefully exiting the program.
            exit(0);
        }
        // Add more handlers for other UCI commands as needed.
    }

    void SendUCICommand(const std::string& command) {
        std::cout << command << std::endl;
        std::cout.flush();
        UCILog(command);
    }

    // Function to send engine info and options.
    void SendUCIInfo() {
        // Send the engine's identity and available options in UCI format.
        // Example:
        std::cout << "id class Class123" << std::endl;
        std::cout << "id author Derek" << std::endl;
        // Add more information and options as needed.
        std::cout << "uciok" << std::endl;
        std::cout.flush();
    }
    Chess* _instance;
    std::string _state;
    bool _running;
};
