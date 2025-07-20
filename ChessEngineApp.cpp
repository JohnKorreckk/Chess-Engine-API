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
 std::string position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
 std::shared_ptr<Board> board = std::make_shared<Board>(boardName, position);
 std::shared_ptr<Engine> engine = std::make_shared<Engine>();

 // Start your command line chess game
 board->Play();

 return true;
}