/**
 * @file ChessEngineApp.h
 * @author John Korreck
 *
 *
 */
 
#ifndef CHESSENGINEAPP_H
#define CHESSENGINEAPP_H

class Board;
class Engine;

class ChessEngineApp : public wxApp {
private:
 bool mIsGameOver = false;
public:
 bool OnInit() override;
 int OnExit() override;
};



#endif //CHESSENGINEAPP_H
