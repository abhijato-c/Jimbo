import chess
import chess.polyglot
import subprocess

eng_old=subprocess.Popen(['./magic'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
eng_new=subprocess.Popen(['./bmi'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
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
    
    eng.stdin.write("position fen " + fen + '\n')
    eng.stdin.write("go movetime 600"+'\n')
    eng.stdin.flush()
    move = eng.stdout.readline()[:-1].split(' ')[1]
    return move

wins=0
draws=0
losses=0
print(len(fens),"fens")
for i in range(100,len(fens)):
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