# Chess Engine API

A production-deployed REST API exposing a C++ AI chess engine to calculate the best move from any given board state in FEN notation. Built with **C++**, **Pybind11**, **FastAPI**, and deployed on **Fly.io**.

---

## Overview

This API allows users to send a board position (in FEN format) and receive the best move calculated by the backend engine using recursive minimax with alpha-beta pruning.

---

## Live API Endpoint

**POST** `https://chess-engine-api.fly.dev/bestmove`

---

## Making a Request

**Method:** `POST`

**URL:** `https://chess-engine-api.fly.dev/bestmove`

### Headers

```
Content-Type: application/json
```

### Request Body

```json
{
  "fen": "your FEN string here"
}
```

#### Example cURL Request

```bash
curl -X POST https://chess-engine-api.fly.dev/bestmove \
  -H "Content-Type: application/json" \
  -d '{"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"}'
```

---

### Response

On success (`200 OK`), returns:

```json
{
  "best_move": "e2e4"
}
```

- **best_move**: The engine’s recommended move in algebraic notation.

---

## Rate Limiting

The API enforces rate limits for cost control:

- Maximum: **10 requests per hour (global)**

Exceeding this limit will return **HTTP 429 Too Many Requests**.

---

## Tech Stack

- **C++**: Core engine implementation with minimax and alpha-beta pruning
- **Pybind11**: Bindings to expose C++ logic as a Python module
- **FastAPI**: Python web server for REST endpoints
- **Docker**: Containerization for build and deployment
- **Fly.io**: Deployment of containerized backend

---

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss proposed updates.