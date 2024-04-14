#include "Application.h"
#include "imgui/imgui.h"
#include "classes/TicTacToe.h"
#include "classes/Chess.h"

namespace ClassGame {
        //
        // our global variables
        //
        Chess *game = nullptr;
        bool gameOver = false;
        int gameWinner = -1;
        bool selectedColor = false;
        int AIPlayer = 1;

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = new Chess();
            game->_gameOptions.AIPlayer = 1;
            game->_gameOptions.rowX = 8;
            game->_gameOptions.rowY = 8;
            game->setUpBoard();
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame() 
        {
#if defined(UCI_INTERFACE)
            if (!selectedColor)
            {
                AIPlayer = 1;
                selectedColor = true;
                game = new Chess();
                game->_gameOptions.AIPlayer = AIPlayer;
                game->setUpBoard();
            }
            if (game->gameHasAI() && game->getCurrentPlayer()->isAIPlayer())
            {
                game->updateAI();
            }
            game->drawFrame();
#else
                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

                ImGui::Begin("Settings");
                ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                std::string state = game->stateString();
                //
                // break state string into 8 rows of 8 characters
                //
                if (state.length() == 128) {
                    for (int y=0; y<8; y++) {
                        std::string row = state.substr(y*16, 16);
                        ImGui::Text("%s", row.c_str());
                    }
                } else {
                        ImGui::Text("%s", state.c_str());
                }
                if (gameOver) {
                    ImGui::Text("Game Over!");
                    ImGui::Text("Winner: %d", gameWinner);
                    if (ImGui::Button("Reset Game")) {
                        game->stopGame();
                        game->setUpBoard();
                        gameOver = false;
                        gameWinner = -1;
                    }
                }
                ImGui::End();

                ImGui::Begin("GameWindow");
                if (game->gameHasAI() && (game->getCurrentPlayer()->isAIPlayer()))
                {
                    game->updateAI();
                }
                game->drawFrame();
                ImGui::End();
#endif
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn() 
        {
            Player *winner = game->checkForWinner();
            if (winner)
            {
                gameOver = true;
                gameWinner = winner->playerNumber();
            }
            if (game->checkForDraw()) {
                gameOver = true;
                gameWinner = -1;
            }
        }
}
