# Chess Engine

A high-performance chess engine implemented in C++ with Python bindings.

## Features

- **Strong AI**: Implements negamax search with alpha-beta pruning
- **Optimization**: Uses transposition tables, move ordering, and quiescence search
- **Opening Book**: Pre-programmed openings from PGN files
- **Evaluation Function**: Comprehensive position evaluation, including:
  - Material balance
  - Piece-square tables
  - Mobility evaluation
  - Pawn structure analysis
  - Bishop pair bonus
  - King safety
- **Python Integration**: Easy to use from Python applications via bridge
- **GUI Ready**: Compatible with the included Python UI

## Project Structure

```
chess-engine/
├── src/                      # C++ source code
│   ├── chess.hpp             # Core chess logic and data structures
│   ├── ChessEngineWrapper.cpp # Interface between C++ engine and Python
│   ├── engine/               # Engine components
│       ├── ChessEngine.cpp   # Main engine implementation
│       ├── ChessEngine.hpp   # Engine class definition
│       ├── Evaluation.cpp    # Position evaluation
│       ├── Evaluation.hpp    # Evaluation parameters and functions
│       ├── OpeningMove.cpp   # Opening book implementation
│       ├── OpeningMove.hpp   # Opening book interface
│       ├── See.hpp           # Static Exchange Evaluation
│       ├── transposition_table.cpp # Transposition table implementation
│       └── transposition_table.hpp # Transposition table interface
├── ui/                       # Python UI components
│   ├── ChessBot.py           # Bot implementation
│   ├── ChessEngineBridge.py  # Bridge between Python and C++ engine
│   ├── ChessGame.py          # Game logic for UI
│   ├── GameMode.py           # Different game modes
│   └── UI.py                 # User interface
├── assets/                   # Game assets
│   ├── fonts/                # UI fonts
│   ├── opening/              # Opening book PGN files
│   ├── sounds/               # Game sound effects
│   └── themes/               # Board and piece themes
├── chess_engine_wrapper.dll  # Compiled engine library (Windows)
└── Makefile                  # Build configuration
```

## Technical Details

### Search Algorithm

The engine uses an iterative deepening negamax search with alpha-beta pruning. Key optimizations include:

- **Transposition Table**: Caches previously evaluated positions
- **Move Ordering**: Orders moves to improve alpha-beta pruning efficiency
- **Quiescence Search**: Extends search in volatile positions to avoid horizon effect
- **Late Move Reduction**: Reduces search depth for less promising moves
- **Static Exchange Evaluation**: Evaluates capture sequences efficiently

### Evaluation Function

Position evaluation considers multiple factors:

- **Material**: Basic piece values with different weights for middlegame/endgame
- **Piece Position**: Square-dependent piece values using piece-square tables
- **Mobility**: Rewards pieces that control more squares
- **Pawn Structure**: Evaluates passed pawns, isolated pawns
- **Bishop Pair**: Gives bonus for having both bishops
- **King Safety**: Evaluates king position relative to the game phase
- **Endgame Knowledge**: Special evaluations for common endgame scenarios

### Opening Book

The engine can use pre-defined opening moves from PGN files:

- Currently configured to use `assets/opening/Adams.pgn`
- Automatically selects the most frequent move for a given position
- Can be disabled with `engine.enableOpeningBook(false)`
## Building

### Prerequisites

- C++17 compatible compiler
- Python 3.x with ctypes
- CMake (optional)

## Acknowledgments

This chess engine uses the chess-library for move generation and board representation.
