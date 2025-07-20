from fastapi import FastAPI, Request
from pydantic import BaseModel
from slowapi import Limiter, _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
import chessengine

app = FastAPI()

# Fix: key_func takes one argument (request)
limiter = Limiter(key_func=lambda request: "global")
app.state.limiter = limiter
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)

class MoveRequest(BaseModel):
    fen: str

engine = chessengine.Engine()

@app.post("/bestmove")
@limiter.limit("10/minute")  # Global limit shared by all requests
async def best_move(request: Request, move_request: MoveRequest):
    board = chessengine.Board("Board", move_request.fen)
    move = engine.find_best_move(board, 3)
    return {"best_move": move}
