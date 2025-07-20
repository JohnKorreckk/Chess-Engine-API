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

class ChessEngineApp {
private:
 bool mIsGameOver = false;
public:
 bool OnInit();
};



#endif //CHESSENGINEAPP_H
