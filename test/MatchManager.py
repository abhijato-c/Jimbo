import chess
import chess.polyglot
import chess.engine

eng_old=chess.engine.SimpleEngine.popen_uci('./EngineOld.out')
eng_new=chess.engine.SimpleEngine.popen_uci('./Engine')
brdr=chess.polyglot.MemoryMappedReader('OpeningBook.bin')

fens=[]
fil=open('./test/fens.txt','r')
for lin in fil:
    fens.append(lin)
fil.close()

def ob(fen):
    o=brdr.get(chess.Board(fen=fen))
    if o:
        o=chess.Board(fen=fen).uci(o.move)
    return o

def BestMove(fen,eng):
    segments = fen.split(' ')
    move = None
    if int(segments[-1])<10:
        move = ob(fen)
    if move:
        return move
    
    board = chess.Board(fen=fen)
    result = eng.play(board, chess.engine.Limit(depth=6))
    move = result.move.uci()
    return move

wins=0
draws=0
losses=0
print(len(fens),"fens")
for i in range(200,len(fens)):
    fen=fens[i][:-1]
    print("game",i+1,"/",len(fens))
    print()
    b=chess.Board(fen=fen)
    while(not b.is_game_over()):
        if b.turn==chess.WHITE:
            b.push(b.parse_uci(BestMove(b.fen(),eng_old)))
        else:
            b.push(b.parse_uci(BestMove(b.fen(),eng_new)))
    outcome = b.outcome()
    if outcome:
        if outcome.winner == chess.WHITE:
            losses+=1
        elif outcome.winner == chess.BLACK:
            wins+=1
        else:
            draws+=1
    b=chess.Board(fen=fen)
    while(not b.is_game_over()):
        if b.turn==chess.WHITE:
            b.push(b.parse_uci(BestMove(b.fen(),eng_new)))
        else:
            b.push(b.parse_uci(BestMove(b.fen(),eng_old)))
    outcome = b.outcome()
    if outcome:
        if outcome.winner == chess.WHITE:
            wins+=1
        elif outcome.winner == chess.BLACK:
            losses+=1
        else:
            draws+=1
    print("wins",wins)
    print("draws",draws)
    print("losses",losses)
    print()