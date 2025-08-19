/**
 * @file ChessEngineApp.cpp
 * @author John Korreck
 */

#include "pch.h"
#include <algorithm>

#include "ChessEngineApp.h"
#include "Board.h"
#include "Engine.h"

/**
 * Initialize the application.
 * @return True if successful
 */
bool ChessEngineApp::OnInit()
{
 std::string boardName = "Board";
 std::string position = "1nr1r3/n4Q2/P1kp2N1/2p3B1/1pp3P1/6P1/1R2P2R/K5N1 w - - 3 43";
 std::shared_ptr<Board> board = std::make_shared<Board>(boardName, position);
 std::shared_ptr<Engine> engine = std::make_shared<Engine>();

 // Start your command line chess game
 board->Play();

 return true;
}