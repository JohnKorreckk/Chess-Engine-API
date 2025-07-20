from fastapi import FastAPI
from pydantic import BaseModel
import chessengine

# Initialize FastAPI app
app = FastAPI()

# Define request schema
class MoveRequest(BaseModel):
    fen: str

# Initialize Engine once
engine = chessengine.Engine()

@app.post("/bestmove")
async def best_move(request: MoveRequest):
    # Create Board from FEN
    board = chessengine.Board("Board", request.fen)
    # Find best move using C++ Engine.
    move = engine.find_best_move(board, 3)
    return {"best_move": move}