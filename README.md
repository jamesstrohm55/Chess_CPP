# Chess Engine

A fully playable chess engine with both a console interface and an SDL2 graphical GUI, written in modern C++17.

## Features

- **Complete Chess Rules** — All standard moves including castling, en passant, pawn promotion, and the 50-move draw rule
- **Legal Move Generation** — Pseudo-legal generation with legality filtering for check, checkmate, and stalemate detection
- **Game State Management** — Full make/unmake move support with state history for undo
- **Console Interface** — Interactive text-based game with Unicode piece rendering, supporting both UCI (`e2e4`) and algebraic (`Nf3`, `O-O`) notation
- **Graphical GUI** — SDL2-based board with click-to-move interaction, move highlighting, and a promotion dialog
- **Draw Detection** — 50-move rule and insufficient material (K vs K, K+B vs K, K+N vs K)

## Architecture

```
Layer 4:  GUI (SDL2)          -- Rendering, mouse input, piece sprites
Layer 3:  Game / Game Loop    -- Turn management, input parsing, game-over detection
Layer 2:  MoveGenerator       -- Legal move generation, check/mate/stalemate
Layer 1:  Board + GameState   -- Make/unmake moves, attack detection
Layer 0:  Piece, Move, Square -- Core data types
```

## Project Structure

```
Chess/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # Entry point (console or GUI mode)
│   ├── Board.cpp             # Board, GameState, make/unmake, attack detection
│   ├── MoveGenerator.cpp     # Legal and pseudo-legal move generation
│   ├── Game.cpp              # Console game loop and input parsing
│   └── GUI.cpp               # SDL2 rendering and mouse interaction
├── include/
│   ├── Piece.h               # Piece/Color enums, display chars, Unicode helpers
│   ├── Board.h               # Board class, GameState struct, Square struct
│   ├── Move.h                # Move struct with special move flags
│   ├── MoveGenerator.h       # Move generator with per-piece generation
│   ├── Game.h                # Game class with turn management
│   └── GUI.h                 # SDL2 GUI class
└── assets/
    └── pieces/               # 12 PNG piece sprites (wP.png, bK.png, etc.)
```

## Building

Requires CMake 3.16+ and a C++17 compiler.

### Console only

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Debug
```

### With GUI (requires SDL2 via vcpkg)

```bash
vcpkg install sdl2 sdl2-image --triplet=x64-windows
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```

## Running

```bash
# Console mode
./build/Debug/Chess.exe

# GUI mode (requires SDL2 build)
./build/Debug/ChessGUI.exe --gui
```

## Console Commands

| Command   | Action                        |
|-----------|-------------------------------|
| `e2e4`    | Move in UCI notation          |
| `Nf3`    | Move in algebraic notation    |
| `O-O`    | Kingside castle               |
| `O-O-O`  | Queenside castle              |
| `undo`   | Take back the last move       |
| `moves`  | List all legal moves          |
| `resign` | Resign the game               |
| `draw`   | Claim a draw                  |
| `quit`   | Exit the program              |

## GUI Controls

- **Click** a piece to select it and see highlighted legal destinations
- **Click** a highlighted square to make the move
- **Click** elsewhere to deselect
- On **pawn promotion**, an overlay appears to choose the promoted piece
