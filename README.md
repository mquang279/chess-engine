# ♟️ Chess Engine
This is a project for the Artificial Intelligence course, carried out by a team of four members.
- 23021641 Lê Hoài Nam
- 23021669 Bùi Minh Quang
- 23021673 Phạm Minh Quân
- 23021629 Nguyễn Giang Minh

The product is a chess-playing engine developed in C++ and Python, supporting the UCI (Universal Chess Interface) protocol. It implements the Negamax algorithm with various enhancements such as Alpha-Beta Pruning, Late Move Reductions (LMR), and more. The project aims to build a powerful, extensible engine that can be easily integrated into user interfaces or automated systems.

## 🚀Key Features
- **Search Algorithm**: Negamax combined with Alpha-Beta Pruning and LMR to optimize the performance of finding the best move.
- **Transposition Table**: Stores and retrieves previously evaluated positions, minimizing redundant calculations and improving efficiency.
- **Position Evaluation**: Evaluates positions using factors such as piece values, pawn structure, control of the center, and king safety.
- **Performance Optimization**: Utilizes Bitboards and Zobrist Hashing to accelerate processing speed and reduce memory usage.
- **UCI Protocol Support**: Easily integrates with popular user interfaces like Arena, CuteChess, or testing tools such as CuteChess-cli.

