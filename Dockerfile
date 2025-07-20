# Build stage
FROM python:3.12-slim AS builder

# Install build tools
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    git \
 && rm -rf /var/lib/apt/lists/*

# Install pybind11 (needed for build)
RUN pip install pybind11

# Set workdir
WORKDIR /app

# Copy source code
COPY . .

# Clean any previous build cache
RUN rm -rf build

# Build C++ code and bindings
RUN cmake -S . -B build && cmake --build build

# Copy the built pybind11 module to /app root for Python import
RUN cp build/chessengine*.so .

# -------------------------

# Final runtime stage
FROM python:3.12-slim

# Install FastAPI and Uvicorn
RUN pip install fastapi uvicorn slowapi


# Copy built files from builder stage
WORKDIR /app
COPY --from=builder /app /app

# Expose port
EXPOSE 8000

# Run FastAPI server
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]