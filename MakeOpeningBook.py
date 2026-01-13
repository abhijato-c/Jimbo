import chess
import chess.pgn
import chess.polyglot
import struct
import os
from collections import defaultdict

def EncodeMove(move):
    #Encodes a chess.Move object into the 16-bit Polyglot integer format.
    #Bits 0-5: to, 6-11 from, 12-14 promo (None=0, Knight=1, Bishop=2, Rook=3, Queen=4)
    
    to_sq = move.to_square
    from_sq = move.from_square
    promo = 0
    if move.promotion:
        promo = move.promotion - 1
        
    return (promo << 12) | (from_sq << 6) | to_sq

def CreateBook(pgn_paths, output_path, depth_limit):
    print(f"Processing")

    # Dictionary structure: BookEntries[zobrist_key][move_encoded] = count
    BookEntries = defaultdict(lambda: defaultdict(int))
    TotalGames = 0
    
    # Parse Games
    for file_path in pgn_paths:
        with open(file_path, "r", encoding="utf-8") as pgn_f:
            while True:
                try: game = chess.pgn.read_game(pgn_f)
                except: continue
                if not game: break

                TotalGames += 1
                board = game.board()
                
                # Iterate through moves
                for move in game.mainline_moves():
                    if board.ply() > depth_limit: break
                    zobrist_key = chess.polyglot.zobrist_hash(board)
                    BookEntries[zobrist_key][move] += 1
                    board.push(move)

                if(TotalGames%500==0): print(f"Processed {TotalGames} games")

    print(f"Scanned {TotalGames} games.")
    print(f"Found {len(BookEntries)} unique positions within depth limit.")

    # Format data
    BinaryEntries = []

    for key, moves in BookEntries.items():
        MaxWeight = max(moves.values())
        for move, weight in moves.items():
            normalized = int(weight / MaxWeight * 65535)
            if normalized == 0: continue

            BinaryEntries.append({
                "key": key,
                "move": EncodeMove(move),
                "weight": normalized,
                "learn": 0
            })

    # Sort entries
    print("Sorting entries")
    BinaryEntries.sort(key=lambda x: x["key"])

    # write file
    # Polyglot Entry Format:
    # - Key:    8 bytes unsigned long long
    # - Move:   2 bytes unsigned short
    # - Weight: 2 bytes unsigned short
    # - Learn:  4 bytes unsigned int
    
    print(f"Writing to {output_path}")
    with open(output_path, "wb") as f:
        for entry in BinaryEntries:
            data = struct.pack(
                ">QHH I",  # Big-endian format string
                entry["key"],
                entry["move"],
                entry["weight"],
                entry["learn"]
            )
            f.write(data)

    print("Done! Book created successfully.")

if __name__ == "__main__":
    CreateBook(['pgns/'+x for x in os.listdir('pgns')], 'ob.bin', 20)