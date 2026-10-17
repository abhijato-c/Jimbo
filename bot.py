import chess
import chess.polyglot
import requests
import json
import backoff
import subprocess

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

ChallengeCooldown = 5
LastChallengeHit = 0
MoveCooldown = 0.1
LastMoveHit = 0

token=str(open('token.txt','r').read().strip())
header={"Authorization": f"Bearer {token}"}

@backoff.on_exception(backoff.expo, Exception, max_tries=5)
def post(endpoint,args=[]):
    if len(args)==2:
        url=ENDPOINTS[endpoint].format(args[0],args[1])
    elif len(args)==1:
        url=ENDPOINTS[endpoint].format(args[0])
    else:
        url=ENDPOINTS[endpoint]
    try:
        return requests.post(url, headers=header)
    except Exception as e:
        print("Skipping POST, error: ",e)

@backoff.on_exception(backoff.expo, Exception, max_tries=5)
def get(endpoint,args=[]):
    if len(args)==2:
        url=ENDPOINTS[endpoint].format(args[0],args[1])
    elif len(args)==1:
        url=ENDPOINTS[endpoint].format(args[0])
    else:
        url=ENDPOINTS[endpoint]
    try:
        return requests.get(url, headers=header)
    except Exception as e:
        print("Skipping GET, error: ",e)

def OngoingGames():
    return json.loads(get('playing').text)['nowPlaying']

def PlayMove(GameID,move):
    return post('move',args=[GameID,move])

def OpeningBookMove(fen):
    o = OpeningBook.get(chess.Board(fen=fen))
    if o:
        return chess.Board(fen=fen).uci(o.move)
    else:
        return None

def BestMove(fen):
    fenb=fen
    fen=fen.split(' ')
    move = None
    if int(fen[-1])<10:
        move = OpeningBookMove(fenb)
    if move:
        print("OpeningBook: "+move)
        return move
    
    eng.stdin.write("position fen " + fen[0]+" "+fen[1] + '\n')
    eng.stdin.write("go movetime 3000"+'\n')
    eng.stdin.flush()
    move = eng.stdout.readline()[:-1].split(' ')[1]

    print("Engine: "+move)
    return move

def GetChallenges():
    return [x['id'] for x in get("listchlng").json()['in']]


if __name__ == '__main__':
    print('Spawning engine and reading opening book', end='   ')
    eng = subprocess.Popen(['./Engine.out'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    OpeningBook = chess.polyglot.MemoryMappedReader('OpeningBook.bin')
    print('Done!')

    print("Your bot is online and ready to accept challenges!")
    while True:
        # Accept Challenges
        for challenge in GetChallenges():
            post('accept',[challenge])
            print('Accepted challenge ', challenge)
        
        # Play moves
        for game in [x for x in OngoingGames() if x['isMyTurn']]:
            print('Game: ' + game['gameId'] + ' FEN: ' + game['fen'])
            move = BestMove(game['fen'])
            PlayMove(game['gameId'], move)