import chess
import chess.polyglot
import requests
import backoff
import json
from pathlib import Path
import subprocess
import time
import platform

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

ENGINE_PATH = ''
if platform.system() == 'Windows':
    ENGINE_PATH = './Engine.exe'
else:
    ENGINE_PATH = './Engine'

OPENING_BOOK_PATH = 'OpeningBook.bin'
TOKEN_FILE_PATH = 'token.txt'

ChallengeCooldown = 5
ChallengeHit = 0
MoveCooldown = 1
MoveHit = 0

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
    segments = fen.split(' ')
    move = None
    if int(segments[-1])<10:
        move = OpeningBookMove(fen)
    if move:
        print("OpeningBook: "+move)
        return move
    
    eng.stdin.write("position fen " + fen + '\n')
    eng.stdin.write("go movetime 3000"+'\n')
    eng.stdin.flush()
    move = eng.stdout.readline()[:-1].split(' ')[1]

    print("Engine: "+move)
    return move

def GetChallenges():
    return [x['id'] for x in get("listchlng").json()['in'] if x['rated']==False and x['speed']=='correspondence']

def UpgradeAccount():
    return post('upgrade')


if __name__ == '__main__':
    if not Path(TOKEN_FILE_PATH).is_file():
        print(f"Error: Token file '{TOKEN_FILE_PATH}' not found!")
        print("Please create a file named 'token.txt' containing your Lichess Bot API token.")
        exit(1)
    if not Path(ENGINE_PATH).is_file():
        print(f"Error: Engine file '{ENGINE_PATH}' not found!")
        print(f"Please place your chess engine in {ENGINE_PATH}.")
        exit(1)
    if not Path(OPENING_BOOK_PATH).is_file():
        print(f"Warning: Opening book file '{OPENING_BOOK_PATH}' not found!")
        print("The bot will function without an opening book!")
    
    token=str(open(TOKEN_FILE_PATH,'r').read().strip())
    header={"Authorization": f"Bearer {token}"}

    UpgradeAccount()

    print('Setting up bot', end='   ')
    eng = subprocess.Popen([ENGINE_PATH], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    if Path(OPENING_BOOK_PATH).is_file(): OpeningBook = chess.polyglot.MemoryMappedReader(OPENING_BOOK_PATH)
    print('Done!')

    print("Your bot is online and ready to accept challenges!")
    while True:
        # Accept Challenges
        if time.time()-ChallengeHit > ChallengeCooldown:
            ChallengeHit = time.time()
            try:
                for challenge in GetChallenges():
                    post('accept',[challenge])
                    print('Accepted challenge ', challenge)
            except Exception as e:
                pass
        
        # Play moves
        if time.time()-MoveHit > MoveCooldown:
            MoveHit = time.time()
            try:
                for game in [x for x in OngoingGames() if x['isMyTurn']]:
                    print('Game: ' + game['gameId'] + ' FEN: ' + game['fen'])
                    move = BestMove(game['fen'])
                    PlayMove(game['gameId'], move)
            except Exception as e:
                pass