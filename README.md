# Chess Engine

A fully playable chess engine with both a console interface and an SDL2 graphical GUI, written in modern C++17. Play against another human or challenge the built-in AI with configurable difficulty.

## Features

- **Complete Chess Rules** — All standard moves including castling, en passant, pawn promotion, and the 50-move draw rule
- **Legal Move Generation** — Pseudo-legal generation with legality filtering for check, checkmate, and stalemate detection
- **Game State Management** — Full make/unmake move support with state history for undo
- **AI Opponent** — Minimax engine with alpha-beta pruning and piece-square table evaluation
  - **Easy** — Depth 2 search (beginner-friendly)
  - **Medium** — Depth 4 search (casual play)
  - **Hard** — Depth 6 search (challenging)
- **Console Interface** — Interactive text-based game supporting both UCI (`e2e4`) and algebraic (`Nf3`, `O-O`) notation
- **Graphical GUI** — SDL2-based board with click-to-move interaction, move highlighting, and a promotion dialog
- **Draw Detection** — 50-move rule and insufficient material (K vs K, K+B vs K, K+N vs K)

## Architecture

```
Layer 5:  GUI (SDL2)          -- Rendering, mouse input, piece sprites
Layer 4:  AI                  -- Minimax + alpha-beta, position evaluation
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
│   ├── main.cpp              # Entry point, menu, console or GUI mode
│   ├── Board.cpp             # Board, GameState, make/unmake, attack detection
│   ├── MoveGenerator.cpp     # Legal and pseudo-legal move generation
│   ├── Game.cpp              # Game loop, input parsing, CPU turn handling
│   ├── AI.cpp                # Minimax search, evaluation, piece-square tables
│   └── GUI.cpp               # SDL2 rendering and mouse interaction
├── include/
│   ├── Piece.h               # Piece/Color enums, display helpers
│   ├── Board.h               # Board class, GameState struct, Square struct
│   ├── Move.h                # Move struct with special move flags
│   ├── MoveGenerator.h       # Move generator with per-piece generation
│   ├── Game.h                # Game class with turn management and AI integration
│   ├── AI.h                  # AI class with configurable difficulty
│   └── GUI.h                 # SDL2 GUI class
└── assets/
    └── pieces/               # 12 SVG piece sprites (wP.svg, bK.svg, etc.)
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
# Install dependencies
vcpkg install sdl2 sdl2-image --triplet=x64-windows

# Build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Debug
```

## Running

```bash
# Console mode (run from project root)
build\Debug\Chess.exe

# GUI mode (run from project root so assets are found)
build\Debug\ChessGUI.exe --gui
```

On startup, a menu lets you choose:
1. **Human vs Human** or **Human vs CPU**
2. Play as **White** or **Black**
3. Difficulty: **Easy**, **Medium**, or **Hard**

## Console Commands

| Command   | Action                                          |
|-----------|-------------------------------------------------|
| `e2e4`    | Move in UCI notation                            |
| `Nf3`    | Move in algebraic notation                      |
| `O-O`    | Kingside castle                                 |
| `O-O-O`  | Queenside castle                                |
| `e8=Q`   | Pawn promotion                                  |
| `undo`   | Take back the last move (undoes CPU move too)   |
| `moves`  | List all legal moves                            |
| `resign` | Resign the game                                 |
| `help`   | Show available commands                         |
| `quit`   | Exit the program                                |

## GUI Controls

- **Click** a piece to select it and see highlighted legal destinations
- **Click** a highlighted square to make the move
- **Click** elsewhere to deselect
- On **pawn promotion**, an overlay appears to choose the promoted piece
- In CPU mode, the AI moves automatically after your turn

## AI Details

The AI uses a minimax search with alpha-beta pruning for efficient move evaluation. Board positions are scored using:

- **Material values** — Pawn: 100, Knight: 320, Bishop: 330, Rook: 500, Queen: 900
- **Piece-square tables** — Positional bonuses encouraging good piece placement (central control, king safety, pawn advancement)
- **Checkmate scoring** — Prefers faster checkmates over slower ones

## Credits

- Chess piece sprites: [Cburnett set](https://commons.wikimedia.org/wiki/Category:SVG_chess_pieces) (CC BY-SA 3.0), same as used by Lichess
