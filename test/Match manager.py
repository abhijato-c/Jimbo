import chess
import chess.polyglot
import subprocess

<<<<<<< HEAD
eng_old=subprocess.Popen(['./eng_old.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
eng_new=subprocess.Popen(['./eng_new.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
=======
eng_old=subprocess.Popen(['./a.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
eng_new=subprocess.Popen(['./Engine.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
>>>>>>> 5c4f716 (First)
brdr=chess.polyglot.MemoryMappedReader('ob.bin')

fens=[]
fil=open('fens.txt','r')
for lin in fil:
    fens.append(lin)
fil.close()

def ob(fen):
    o=brdr.get(chess.Board(fen=fen))
    if o:
        o=chess.Board(fen=fen).uci(o.move)
    return o

def BestMove(fen,eng):
    fenb=fen
    fen=fen.split(' ')
    if int(fen[-1])<10:
        output=ob(fenb)
        if output:
            return output
    eng.stdin.write(fen[0]+"\n")
    eng.stdin.write(fen[1]+'\n')
    eng.stdin.flush()
    output = eng.stdout.readline()[:-1]
    return output.replace('=','')

wins=0
draws=0
losses=0
print(len(fens),"fens")
for i in range(len(fens)):
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