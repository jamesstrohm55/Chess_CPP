# Chess Engine

A fully playable chess engine with both a console interface and an SDL2 graphical GUI, written in modern C++17. Play against another human or challenge the built-in AI with configurable difficulty.

<!-- Add a screenshot or GIF here for maximum impact:
![Chess GUI](screenshot.png)
-->

## Features

- **Complete Chess Rules** — All standard moves including castling, en passant, pawn promotion, and the 50-move draw rule
- **Legal Move Generation** — Pseudo-legal generation with legality filtering for check, checkmate, and stalemate detection
- **Game State Management** — Full make/unmake move support with state history for undo
- **AI Opponent** — Minimax engine with alpha-beta pruning, quiescence search, and advanced evaluation
  - **Easy** — Depth 2 search (beginner-friendly)
  - **Medium** — Depth 4 search (casual play)
  - **Hard** — Depth 5 search with quiescence (challenging)
- **Graphical GUI** — SDL2-based board with click-to-move interaction, move highlighting, promotion dialog, and in-app menu
- **Live Scoreboard** — Tracks wins, losses, and draws across games in a session
- **Status Panel** — Real-time messages: "CPU is thinking...", "CPU played e2e4. Your turn.", "Check!", "Checkmate!"
- **Console Interface** — Text-based game supporting both UCI (`e2e4`) and algebraic (`Nf3`, `O-O`) notation
- **Draw Detection** — 50-move rule and insufficient material (K vs K, K+B vs K, K+N vs K)

## AI Details

The AI uses a minimax search with alpha-beta pruning. Several techniques work together to make it play strong chess:

| Technique | What it does |
|-----------|-------------|
| **Alpha-Beta Pruning** | Eliminates branches that can't affect the result, dramatically reducing search space |
| **Quiescence Search** | Continues searching capture sequences past the depth limit so the AI doesn't misjudge exchanges |
| **Killer Move Heuristic** | Remembers quiet moves that caused cutoffs and tries them first, speeding up pruning |
| **King Safety** | Penalizes exposed kings, rewards pawn shields, punishes attacked squares near the king |
| **Pawn Structure** | Penalizes doubled and isolated pawns, rewards passed pawns |
| **Bishop Pair Bonus** | Awards a bonus for keeping both bishops |
| **Piece-Square Tables** | Positional bonuses encouraging central control, king safety, and pawn advancement |
| **MVV-LVA Move Ordering** | Sorts captures by most-valuable-victim / least-valuable-attacker for better pruning |

**Material values:** Pawn 100, Knight 320, Bishop 330, Rook 500, Queen 900

## Architecture

```
Layer 5:  GUI (SDL2)          -- Rendering, mouse input, piece sprites, status panel
Layer 4:  AI                  -- Minimax + alpha-beta + quiescence, evaluation
Layer 3:  Game / Game Loop    -- Turn management, input parsing, score tracking
Layer 2:  MoveGenerator       -- Legal move generation, check/mate/stalemate
Layer 1:  Board + GameState   -- Make/unmake moves, attack detection
Layer 0:  Piece, Move, Square -- Core data types
```

## Project Structure

```
Chess/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # Entry point, console or GUI mode
│   ├── Board.cpp             # Board state, make/unmake, attack detection
│   ├── MoveGenerator.cpp     # Legal/pseudo-legal move and capture generation
│   ├── Game.cpp              # Game loop, input parsing, score tracking
│   ├── AI.cpp                # Search engine, evaluation, move ordering
│   └── GUI.cpp               # SDL2 rendering, status panel, menus
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
vcpkg install sdl2 sdl2-image sdl2-ttf --triplet=x64-windows

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
- **Score** and **status messages** are displayed in the panel below the board

## Credits

- Chess piece sprites: [Cburnett set](https://commons.wikimedia.org/wiki/Category:SVG_chess_pieces) (CC BY-SA 3.0), same as used by Lichess
