from fastapi import FastAPI, Request
from pydantic import BaseModel
from slowapi import Limiter, _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
import chessengine

# Initialize FastAPI app
app = FastAPI()

# Create limiter with a global key so all requests share the same bucket
limiter = Limiter(key_func=lambda _: "global")
app.state.limiter = limiter
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)

# Define request schema
class MoveRequest(BaseModel):
    fen: str

# Initialize Engine once
engine = chessengine.Engine()

@app.post("/bestmove")
@limiter.limit("1/minute")  # Global limit: max 100 requests per minute total
async def best_move(request: Request, move_request: MoveRequest):
    # Create Board from FEN
    board = chessengine.Board("Board", move_request.fen)
    # Find best move using C++ Engine.
    move = engine.find_best_move(board, 3)
    return {"best_move": move}