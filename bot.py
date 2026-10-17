import chess.polyglot
import requests
import json
import time
import subprocess
import chess

ENDPOINTS = {
    "profile": "https://lichess.org/api/account",
    "playing": "https://lichess.org/api/account/playing",
    "stream": "https://lichess.org/api/bot/game/stream/{}",
    "stream_event": "https://lichess.org/api/stream/event",
    "move": "https://lichess.org/api/bot/game/{}/move/{}",
    "takeback": "https://lichess.org/api/bot/game/{}/takeback/{}",
    "chat": "https://lichess.org/api/bot/game/{}/chat",
    "abort": "https://lichess.org/api/bot/game/{}/abort",
    "accept": "https://lichess.org/api/challenge/{}/accept",
    "decline": "https://lichess.org/api/challenge/{}/decline",
    "upgrade": "https://lichess.org/api/bot/account/upgrade",
    "resign": "https://lichess.org/api/bot/game/{}/resign",
    "export": "https://lichess.org/game/export/{}",
    "online_bots": "https://lichess.org/api/bot/online",
    "challenge": "https://lichess.org/api/challenge/{}",
    "cancel": "https://lichess.org/api/challenge/{}/cancel",
    "status": "https://lichess.org/api/users/status",
    "public_data": "https://lichess.org/api/user/{}",
    "listchlng": "https://lichess.org/api/challenge"
}
chlng_cldn=5
last_chlng_hit=0
mv_cldn=0.1
last_mv_hit=0
token=str(open('token.txt','r').read().strip())
hdr={"Authorization": f"Bearer {token}"}
def post(endpoint,args=[]):
    if len(args)==2:
        url=ENDPOINTS[endpoint].format(args[0],args[1])
    elif len(args)==1:
        url=ENDPOINTS[endpoint].format(args[0])
    else:
        url=ENDPOINTS[endpoint]
    try:
        return requests.post(url,headers=hdr)
    except Exception as e:
        print("Skipping error:",e.split(':')[0])

def get(endpoint,args=[]):
    if len(args)==2:
        url=ENDPOINTS[endpoint].format(args[0],args[1])
    elif len(args)==1:
        url=ENDPOINTS[endpoint].format(args[0])
    else:
        url=ENDPOINTS[endpoint]
    try:
        return requests.get(url,headers=hdr)
    except Exception as e:
        print("Skipping error:",e.split(':')[0])

def GetOngoingGames():
    return json.loads(get('playing').text)['nowPlaying']

def PlayMove(id,move):
    return post('move',args=[id,move])

def ob(fen):
    o=brdr.get(chess.Board(fen=fen))
    if o:
        o=chess.Board(fen=fen).uci(o.move)
    return o

def BestMove(fen):
    print(fen)
    fenb=fen
    fen=fen.split(' ')
    if int(fen[-1])<10:
        output=ob(fenb)
        if output:
            print("ob:"+output)
            return output
    eng.stdin.write(fen[0]+"\n")
    eng.stdin.write(fen[1]+'\n')
    eng.stdin.flush()
    output = eng.stdout.readline()[:-1]
    print("eng:"+output)
    return output

def GetChallenges():
    return [x['id'] for x in get("listchlng").json()['in']]

eng=subprocess.Popen(['Engine.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
brdr=chess.polyglot.MemoryMappedReader('ob.bin')
while True:
    try:
        if time.time()-last_chlng_hit>chlng_cldn:
            last_chlng_hit=time.time()
            for chlng in GetChallenges():
                post('accept',[chlng])
            chlng_cldn=5
    except Exception as e:
        chlng_cldn+=chlng_cldn/10
        print("Challenge Error cooldown",chlng_cldn)
    try:
        if time.time()-last_mv_hit>mv_cldn:
            last_mv_hit=time.time()
            for game in [x for x in GetOngoingGames() if x['isMyTurn']]:
                mv=BestMove(game['fen'])
                PlayMove(game['gameId'],mv)
            mv_cldn=0.5
    except Exception as e:
        mv_cldn+=mv_cldn/2
        print("Move Error cooldown-",mv_cldn)