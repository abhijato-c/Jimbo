import chess
import chess.engine

eng_old = chess.engine.SimpleEngine.popen_uci('./test/EngineOld')
eng_new = chess.engine.SimpleEngine.popen_uci('./test/Engine')

piece_values = {
    chess.PAWN: 1,
    chess.KNIGHT: 3,
    chess.BISHOP: 3,
    chess.ROOK: 5,
    chess.QUEEN: 9,
    chess.KING: 0,
}

fens = []
fil = open('./test/fens.txt','r')
for lin in fil:
    fens.append(lin)
fil.close()

def BestMove(fen,eng):
    board = chess.Board(fen=fen)
    result = eng.play(board, chess.engine.Limit(time = 1))
    move = result.move.uci()
    return move

w = 0
d = 0
l = 0
a = 0
print(len(fens),"fens")

for i in range(len(fens)):
    fen = fens[i][:-1]
    print("Round - " + str(i+1) + "/" + str(len(fens)))

    # Old as white, new as black
    b = chess.Board(fen=fen)
    while(not b.is_game_over()):
        if b.turn==chess.WHITE:
            best = BestMove(b.fen(), eng_old)
        else:
            best = BestMove(b.fen(), eng_new)
        
        b.push(b.parse_uci(best))

    winner = b.outcome().winner
    if winner == chess.WHITE: l += 1
    elif winner == chess.BLACK: w += 1
    else: d += 1

    white_score = sum(len(b.pieces(piece_type, chess.WHITE)) * val for piece_type, val in piece_values.items())
    black_score = sum(len(b.pieces(piece_type, chess.BLACK)) * val for piece_type, val in piece_values.items())

    a -= white_score 
    a += black_score
    print(b.outcome().termination.name, " ", black_score - white_score)

    # New as white, old as black
    b = chess.Board(fen=fen)
    while(not b.is_game_over()):
        if b.turn==chess.WHITE:
            best = BestMove(b.fen(),eng_new)
        else:
            best = BestMove(b.fen(),eng_old)
        
        b.push(b.parse_uci(best))
    
    winner = b.outcome().winner
    if winner == chess.WHITE: w += 1
    elif winner == chess.BLACK: l += 1
    else: d += 1

    white_score = sum(len(b.pieces(piece_type, chess.WHITE)) * val for piece_type, val in piece_values.items())
    black_score = sum(len(b.pieces(piece_type, chess.BLACK)) * val for piece_type, val in piece_values.items())

    a += white_score 
    a -= black_score
    print(b.outcome().termination.name, " ", white_score - black_score)

    print("wins", w)
    print("draws", d)
    print("losses", l)
    print("adv", a)
    print()

eng_new.close()
eng_new.close()