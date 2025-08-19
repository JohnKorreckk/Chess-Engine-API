from fastapi import FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from slowapi import Limiter, _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
import chessengine

app = FastAPI()

# CORS config - adjust origins as needed
origins = [
    "http://localhost:5173",
    "https://chess-engine-api.netlify.app"
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

limiter = Limiter(key_func=lambda request: "global")
app.state.limiter = limiter
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)

class MoveRequest(BaseModel):
    fen: str

engine = chessengine.Engine()

@app.post("/bestmove")
@limiter.limit("10/minute")
async def best_move(request: Request, move_request: MoveRequest):
    board = chessengine.Board("Board", move_request.fen)
    move = engine.find_best_move(board, 3)
    return {"best_move": move}
