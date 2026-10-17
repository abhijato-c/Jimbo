# Chess Engine & Lichess Bot

A C++, UCI-compatible chess engine featuring an Alpha-Beta pruning search with iterative deepening. This project includes a Python script to deploy the engine as a live bot on Lichess.

## Getting Started

### Compile engine locally

You can download a binary from the releases to use. If you want to compile the engine locally for best performance, use the following command:

```bash
g++ -march=native -Ofast -Wall -Wextra './src/Chess.cpp' -o '.Engine.exe'

```
On linux/mac, remove the .exe from the output.
Go through the UCI command support of the readme to see what commands are available.

### Setting up the Lichess Bot

The code and files required for the Lichess bot is present in the `LichessBot` directory. Instructions here are in reference to that.

1. Ensure you have python installed.
2. Download the packages in `requirements.txt`
3. Make a new Lichess account and create an API token with all 'Bot' permissions. This token will be pasted in the `token.txt` file. For help, just look up how to make a lichess bot account. There is already extensive documentation on that.
4. Create a file named `token.txt` in the directory.
5. Paste your Lichess API token into `token.txt`.
7. Place your engine and opening book in the same directory as the script, and ensure they are named `Engine` and `OpeningBook.bin` respectively. You can get the opening book from this repo, and just download a suitable binary and remane it to `Engine`. You can change these names inside the script, if you wish to.
6. Run the bot script:
```bash
python3 LichessBot/LichessBot.py

```

## UCI Command Support

| Command | Description |
| --- | --- |
| `uci` | Returns `uciok`, signifying the engine is ready. |
| `isready` | Returns `readyok`, no difference compared to `uci` command. |
| `position [fen / startpos]` | Sets up the board state. `moves` command not supported yet. |
| `go [movetime / depth]` | Starts the engine calculation. If multiple constratints are provided, search stops at whichever is reached first. |
| `stop` | Terminates the search and instantly returns the current best move. |
| `quit` | Exits the engine. |

## Engine Features

### Search Algorithms

* **Iterative Deepening**: Dynamically manages time by searching deeper plies until the time limit or max depth is reached.
* **MiniMax with Alpha-Beta Pruning**: Efficiently prunes the search tree to explore deeper lines by eliminating branches that cannot improve the outcome.
* **Move Ordering**: Uses static evaluations to order moves, significantly increasing the efficiency of the Alpha-Beta pruning.

### Technical Specifications

* **UCI Support**: Supports standard commands including `uci`, `isready`, `position` (FEN and startpos), and `go` (depth and movetime).
* **Performance Tools**: Includes a built-in `profile()` function to measure Nodes Per Second (NPS) and average time per move.
* **Lookup Tables**: Utilizes pre-generated tables for sliders (Rooks and Bishops) to accelerate move generation.

## Project Structure

* `src/Chess.cpp`: The UCI loop.
* `src/MakeMove.h`: Search logic(iterative deepening and minimax).
* `src/Defs.h`: Global variable and utility function definitions.
* `src/MoveGen.h`: Move generator.
* `src/MovePiece.h`: Logic for updating board state after moves.
* `src/LookupTables.h`: Initializes the Magic/Pext bitboards for sliding piece lookups.
* `src/Misc.h`: Printing moves, parsing moves, Parse FEN, etc.

* `Lichess Bot/LichessBot.py`: Python script for Lichess API integration.
* `Lichess Bot/OpeningBook.bin`: The [Cerebellum](https://zipproth.de/Brainfish/Cerebellum_Light_3Merge_200916.7z) Opening Book. This is managed by git LFS. Not connected to the engine, it is handled independantly by `LichessBot.py`.
* `Lichess Bot/requirements.txt`: Python pip requirements file to run the bot.
* `Lichess Bot/token.txt`: (User-provided) API token for Lichess.

* `MakeOpeningBook.py`: Makes a polyglot opening book from the PGN files in the `pgns` directory. NOTE - You will have to download your own PGN files in the `pgns` directory if you want to make your own opening book.
* `test/`: A folder containing a script to check if the new version of the engine is actually better.