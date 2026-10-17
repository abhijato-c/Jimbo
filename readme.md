# Chess Engine & Lichess Bot

A C++ chess engine featuring an Alpha-Beta pruning search with iterative deepening and a (minimal, under dev)Universal Chess Interface (UCI). This project includes a Python wrapper to deploy the engine as a live bot on Lichess.

## 🚀 Getting Started

### Prerequisites

* **C++ Compiler**: A modern compiler supporting C++11 or higher (e.g., `g++`).
* **Python 3.x**: Required for the Lichess integration.
* **Lichess Bot Token**: A personal API token from Lichess with "Play as a bot" permissions.

### Installation & Compilation

To achieve maximum performance, the engine should be compiled with aggressive optimizations. Use the following command:

```bash
g++ --param max-inline-insns-single=10000 --param large-function-growth=1000 -march=native -Ofast -Wall -Wextra './src/Chess.cpp' -o '.Engine.out'

```

### Setting up the Lichess Bot

1. Create a file named `token.txt` in the root directory.
2. Paste your Lichess API token into `token.txt`.
3. Run the bot script:
```bash
python3 bot.py

```



## 🧠 Engine Features

### Search Algorithms

* **Iterative Deepening**: Dynamically manages time by searching deeper plies until the time limit (`movetime`) is reached.
* **MiniMax with Alpha-Beta Pruning**: Efficiently prunes the search tree to explore deeper lines by eliminating branches that cannot improve the outcome.
* **Move Ordering**: Uses static evaluations to order moves, significantly increasing the efficiency of the Alpha-Beta pruning.

### Technical Specifications

* **UCI Support**: Supports standard commands including `uci`, `isready`, `position` (FEN and startpos), and `go` (depth and movetime).
* **Performance Tools**: Includes a built-in `profile()` function to measure Nodes Per Second (NPS) and average time per move.
* **Lookup Tables**: Utilizes pre-generated tables for sliders (Rooks and Bishops) to accelerate move generation.

## 🛠 UCI Command Support

| Command | Description |
| --- | --- |
| `uci` | Identifies the engine and returns `uciok`. |
| `isready` | Synchronizes the engine with the GUI. |
| `position [fen / startpos]` | Sets up the board state. |
| `go [movetime / depth]` | Starts the engine calculation. |
| `quit` | Exits the engine. |

## 📂 Project Structure

* `src/Chess.cpp`: The main engine logic, search routines, and UCI loop.
* `src/Init.cpp`: Initialization routines.
* `src/MoveGen.cpp`: Pseudo-legal move generation.
* `src/MovePiece.cpp`: Logic for updating board state after moves.
* `src/LookupTables.cpp`: Magic bitboard or sliding piece lookup generation.
* `src/Misc.cpp`: Printing moves, Parse FEN, etc.
* `pgns/*.pgn`: 249 PGN files downloaded from PGNMentor to make an opening book.
* `test/`: A folder containing a script to check if the new version of the engine is actually better.
* `LichessBot.py`: Python script for Lichess API integration.
* `MakeOpeningBook.py`: Makes a polyglot opening book from the PGN files in the 'pgns' directory.
* `OpeningBook.bin`: An opening book that bot.py uses(not connected to the engine).
* `token.txt`: (User-provided) API token for Lichess.