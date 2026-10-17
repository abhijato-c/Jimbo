import chess
import chess.engine

eng_old=chess.engine.SimpleEngine.popen_uci('./EngineOld')
eng_new=chess.engine.SimpleEngine.popen_uci('./Engine')

fens=[]
fil=open('./test/fens.txt','r')
for lin in fil:
    fens.append(lin)
fil.close()

def BestMove(fen,eng):
    board = chess.Board(fen=fen)
    result = eng.play(board, chess.engine.Limit(depth=6))
    move = result.move.uci()
    return move

wins=0
draws=0
losses=0
print(len(fens),"fens")

for i in range(500,len(fens)):
    fen=fens[i][:-1]
    print("game",i+1,"/",len(fens))
    print()

    # Old as white, new as black
    b=chess.Board(fen=fen)
    while(not b.is_game_over()):
        if b.turn==chess.WHITE:
            b.push(b.parse_uci(BestMove(b.fen(),eng_old)))
        else:
            b.push(b.parse_uci(BestMove(b.fen(),eng_new)))

    winner = b.outcome().winner
    if winner == chess.WHITE: losses+=1
    elif winner == chess.BLACK: wins+=1
    else: draws+=1

    # New as white, old as black
    b=chess.Board(fen=fen)
    while(not b.is_game_over()):
        if b.turn==chess.WHITE:
            b.push(b.parse_uci(BestMove(b.fen(),eng_new)))
        else:
            b.push(b.parse_uci(BestMove(b.fen(),eng_old)))
    
    winner = b.outcome().winner
    if winner == chess.WHITE: wins+=1
    elif winner == chess.BLACK: losses+=1
    else: draws+=1

    print("wins",wins)
    print("draws",draws)
    print("losses",losses)
    print()