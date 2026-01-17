# Chess Engine & Lichess Bot

A C++ chess engine featuring an Alpha-Beta pruning search with iterative deepening and a (minimal, under dev)Universal Chess Interface (UCI). This project includes a Python wrapper to deploy the engine as a live bot on Lichess.

## 🚀 Getting Started

### Compile engine locally

You can download a binary from the releases to use. If you want to compile the engine locally for best performance, use the following command:

```bash
g++ -march=native -Ofast -Wall -Wextra './src/Chess.cpp' -o '.Engine.exe'

```
On linux/mac, remove the .exe from the output.
Go through the UCI command support of the readme to see what commands are available, since not all commands are supported yet.

### Setting up the Lichess Bot

The code and files required for the Lichess bot is present in the `Lichess Bot` directory. Instructions here are in reference to that.

1. Ensure you have python installed.
2. Download the packages in `requirements.txt`
3. Make a new Lichess account and create an API token with all 'Bot' permissions. This token will be pasted in the `token.txt` file. For help, just look up how to make a lichess bot account. There is already extensive documentation on that.
4. Create a file named `token.txt` in the directory.
5. Paste your Lichess API token into `token.txt`.
7. Place your engine and opening book in the same directory as the script, and ensure they are named `Engine` and `OpeningBook.bin` respectively. You can get the opening book from this repo, and just download a suitable binary and remane it to `Engine`. You can change these names inside the script, if you wish to.
6. Run the bot script:
```bash
python3 LichessBot.py

```

## 🛠 UCI Command Support

| Command | Description |
| --- | --- |
| `uci` | Returns `uciok`, signifying the engine is ready. |
| `isready` | Synchronizes the engine with the GUI. |
| `position [fen / startpos]` | Sets up the board state. `moves` command not supported |
| `go [movetime / depth]` | Starts the engine calculation. Use either movetime or depth, not both together. Multiple constraints not supported. |
| `quit` | Exits the engine. |

`stop` command not supported.

## 🧠 Engine Features

### Search Algorithms

* **Iterative Deepening**: Dynamically manages time by searching deeper plies until the time limit (`movetime`) is reached.
* **MiniMax with Alpha-Beta Pruning**: Efficiently prunes the search tree to explore deeper lines by eliminating branches that cannot improve the outcome.
* **Move Ordering**: Uses static evaluations to order moves, significantly increasing the efficiency of the Alpha-Beta pruning.

### Technical Specifications

* **UCI Support**: Supports standard commands including `uci`, `isready`, `position` (FEN and startpos), and `go` (depth and movetime).
* **Performance Tools**: Includes a built-in `profile()` function to measure Nodes Per Second (NPS) and average time per move.
* **Lookup Tables**: Utilizes pre-generated tables for sliders (Rooks and Bishops) to accelerate move generation.

## 📂 Project Structure

* `src/Chess.cpp`: The main engine logic, search routines, and UCI loop.
* `src/Init.cpp`: Initialization routines.
* `src/MoveGen.cpp`: Pseudo-legal move generation.
* `src/MovePiece.cpp`: Logic for updating board state after moves.
* `src/LookupTables.cpp`: Magic bitboard or sliding piece lookup generation.
* `src/Misc.cpp`: Printing moves, Parse FEN, etc.
* `Lichess Bot/LichessBot.py`: Python script for Lichess API integration.
* `Lichess Bot/OpeningBook.bin`: An opening book that bot.py uses(not connected to the engine).
* `Lichess Bot/requirements.txt`: Python pip requirements file to run the bot.
* `Lichess Bot/token.txt`: (User-provided) API token for Lichess.
* `MakeOpeningBook.py`: Makes a polyglot opening book from the PGN files in the 'pgns' directory.
* `pgns/*.pgn`: 249 PGN files downloaded from PGNMentor to make an opening book.
* `test/`: A folder containing a script to check if the new version of the engine is actually better.