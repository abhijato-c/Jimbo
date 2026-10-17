import logging
import platform
import time
import json
import sys
import threading
from pathlib import Path
from typing import Optional, List, Dict, Any

import chess
import chess.polyglot
import chess.engine
import requests
import backoff

BASE_URL = "https://lichess.org/"
LOG_LEVEL = logging.INFO
TOKEN_FILE_PATH = Path('Lichess Bot/token.txt')
OPENING_BOOK_PATH = Path('Lichess Bot/OpeningBook.bin')
ENDPOINTS = {
    "profile": "api/account",
    "playing": "api/account/playing",
    "stream": "api/bot/game/stream/{}",
    "stream_event": "api/stream/event",
    "move": "api/bot/game/{}/move/{}",
    "takeback": "api/bot/game/{}/takeback/{}",
    "chat": "api/bot/game/{}/chat",
    "abort": "api/bot/game/{}/abort",
    "accept": "api/challenge/{}/accept",
    "decline": "api/challenge/{}/decline",
    "upgrade": "api/bot/account/upgrade",
    "resign": "api/bot/game/{}/resign",
    "export": "game/export/{}",
    "online_bots": "api/bot/online",
    "challenge": "api/challenge/{}",
    "cancel": "api/challenge/{}/cancel",
    "status": "api/users/status",
    "public_data": "api/user/{}",
    "listchlng": "api/challenge"
}

if platform.system() == 'Windows':
    ENGINE_PATH = Path('./Lichess Bot/Engine.exe')
else:
    ENGINE_PATH = Path('./Lichess Bot/Engine')

logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level=LOG_LEVEL,  datefmt='%H:%M:%S', force=True)
logger = logging.getLogger(__name__)


class LichessBot:
    def __init__(self, TokenPath: Path, EnginePath: Path, OpeningBookPath: Path):
        self.token = self.LoadToken(TokenPath)
        self.EnginePath = EnginePath
        self.OpeningBookPath = OpeningBookPath
        
        self.session = requests.Session()
        self.session.headers.update({"Authorization": f"Bearer {self.token}"})
        
        self.OpeningBook: Optional[chess.polyglot.MemoryMappedReader] = None
        self.Username: str = ""

        # Timers
        self.last_challenge_check = 0.0
        self.last_move_check = 0.0
        self.challenge_cooldown = 5.0
        self.move_cooldown = 1.0

        # Thread Management
        self.ActiveEngines: List[chess.engine.SimpleEngine] = []
        self.EnginesLock = threading.Lock()

        self.InitResources()

    @staticmethod
    def LoadToken(path: Path) -> str:
        if not path.is_file():
            logger.error(f"Token file '{path}' not found.")
            sys.exit(1)
        return path.read_text().strip()
    
    def IsBotAcctount(self) -> bool:
        data = self._request('GET', 'profile').json()
        return data.get('title', "") == 'BOT'

    # Load resources
    def InitResources(self):
        # Check Engine
        if not self.EnginePath.is_file():
            logger.error(f"Engine file '{self.EnginePath}' not found.")
            sys.exit(1)

        # Check Opening Book
        if self.OpeningBookPath.is_file():
            try:
                self.OpeningBook = chess.polyglot.MemoryMappedReader(str(self.OpeningBookPath))
                logger.info("Opening book loaded.")
            except Exception as e:
                logger.warning(f"Could not load opening book: {e}")
        else:
            logger.warning("No opening book found. Running without one.")

        # Upgrade acct
        if not self.IsBotAcctount():
            logger.info("Upgrading account to bot status...")
            self.UpgradeAccount()
        
        # Get username
        self.Username = self._request('GET', 'profile').json().get('id')
        
        logger.info(f"Bot {self.Username} is online and ready!")
    
    def SpawnEngine(self) -> chess.engine.SimpleEngine:
        try:
            logger.info("Starting Chess Engine...")
            Engine = chess.engine.SimpleEngine.popen_uci('./' + str(self.EnginePath))
            return Engine
        except Exception as e:
            logger.error(f"Failed to start engine: {e}")
            sys.exit(1)

    def stop(self):
        if self.OpeningBook: self.OpeningBook.close()
        with self.EnginesLock:
            for engine in self.ActiveEngines: engine.quit()
            self.ActiveEngines.clear()
        
        logger.info("Bot shut down.")

    def _log_backoff(details):
        logger.warning(f"Backing off {details['wait']:.1f}s after {details['tries']} tries. Error: {details['exception']}")

    @backoff.on_exception(backoff.expo, Exception, max_tries=5, on_backoff=_log_backoff)
    def _request(self, method: str, endpoint_key: str, *args, **kwargs) -> Optional[requests.Response]:
        url = BASE_URL + ENDPOINTS[endpoint_key].format(*args)
        response = self.session.request(method, url, **kwargs)
        response.raise_for_status()
        return response

    def UpgradeAccount(self):
        self._request('POST', 'upgrade')

    def GetChallenges(self) -> List[str]:
        response = self._request('GET', 'listchlng')
        if not response: return []
        return [c['id'] for c in response.json().get('in', []) if not c.get('rated', False)]

    def AcceptChallenge(self, id: str):
        if self._request('POST', 'accept', id): logger.info(f"Accepted challenge: {id}")

    def RejectChallenge(self, id: str, reason: str = "generic"):
        if self._request('POST', 'decline', id, data={"reason": reason}): logger.info(f"Declined challenge: {id} | Reason: {reason}")

    def GetOngoingGames(self) -> List[Dict[str, Any]]:
        response = self._request('GET', 'playing')
        if not response: return []
        return response.json().get('nowPlaying', [])

    def CalculateTimeLimit(self, SecondsLeft: int, MovesPlayed: int) -> float:
        if MovesPlayed < 60: MovesLeft = 80 - MovesPlayed
        else: MovesLeft = 20
        Time = max(0.2, SecondsLeft / MovesLeft - 0.2)
        if Time > 10.0: Time = 5.0
        return Time

    def GetBestMove(self, fen: str, TimeLimit: float, Engine: chess.engine.SimpleEngine) -> str:
        board = chess.Board(fen)

        # Opening Book
        if self.OpeningBook:
            try:
                entry = self.OpeningBook.weighted_choice(board)
                logger.info(f"Book Move: {entry.move.uci()}")
                return entry.move.uci()
            except IndexError:
                pass

        # Engine
        try:
            # limit is in seconds
            result = Engine.play(board, chess.engine.Limit(time=TimeLimit))
            if result.move:
                logger.info(f"Engine Move: {result.move.uci()}")
                return result.move.uci()
        except Exception as e:
            logger.error(f"Engine error: {e}")
        
        return ""
    
    def PlayGame(self, GameID: str):
        url = BASE_URL + ENDPOINTS['stream'].format(GameID)
        Color = None
        logger.info(f"Streaming Game: {GameID}")
        Engine = self.SpawnEngine()
        with self.EnginesLock: self.ActiveEngines.append(Engine)

        with self.session.get(url, stream=True) as response:
            for line in response.iter_lines():
                # Grab event
                if not line: continue
                try: event = json.loads(line.decode('utf-8'))
                except json.JSONDecodeError: continue

                EventType = event.get('type')

                # Initial game state
                if EventType == 'gameFull':
                    Color = chess.WHITE if event['white'].get('id') == self.Username else chess.BLACK
                    State = event['state']

                # Move made
                elif EventType == 'gameState': State = event

                # Other events
                else: continue

                # Check game status. Is it over?
                Status = State.get('status')
                if Status != 'started':
                    logger.info(f"Game {GameID} ended with status: {Status}")
                    break
                
                # Set up board
                moves = State.get('moves')
                board = chess.Board()
                for move in moves.split(): board.push_uci(move)
                
                # Skip if not our turn
                if board.turn != Color: continue

                # Compute and Send Move
                TimeLeft = State['wtime'] if Color == chess.WHITE else State['btime']
                limit = self.CalculateTimeLimit(TimeLeft / 1000, len(board.move_stack))
                logger.info(f"Game {GameID} | Time: {limit:.2f}s")
                BestMove = self.GetBestMove(board.fen(), limit, Engine)
                if BestMove: self._request('POST', 'move', GameID, BestMove)
        
        logger.info(f"Stopped stream: {GameID}")
        with self.EnginesLock:
            self.ActiveEngines.remove(Engine)
        Engine.quit()

    def MainLoop(self):
        url = BASE_URL + ENDPOINTS['stream_event']
        
        while True:
            try:
                with self.session.get(url, stream=True) as response:
                    if response.status_code != 200:
                        logger.error(f"Event stream failed: {response.status_code}")
                        time.sleep(5)
                        continue

                    for line in response.iter_lines():
                        # Grab event
                        if not line: continue
                        try: event = json.loads(line.decode('utf-8'))
                        except json.JSONDecodeError: continue

                        # Handle Incoming Challenges
                        if event['type'] == 'challenge':
                            event = event['challenge']
                            if event['destUser']['id'] != self.Username: continue # Ignore outgoing

                            logger.info(f"Incoming challenge from {event['challenger']['id']}")
                            if not event.get('rated'): self.AcceptChallenge(event['id'])
                            else: self.RejectChallenge(event['id'], reason="casual")
                        
                        # Handle Game Start
                        elif event['type'] == 'gameStart':
                            event = event['game']
                            id = event['id']
                            logger.info(f"Game started: {id}")
                            t = threading.Thread(target=self.PlayGame, args=(id,), daemon=True)
                            t.start()
                        
                        elif event['type'] == 'gameFinish':
                            logger.info(f"Game finished: {event['game']['id']}")

            except requests.RequestException as e:
                logger.error(f"Stream disconnected: {e}. Reconnecting in 5s...")
                time.sleep(5)
            except KeyboardInterrupt:
                self.stop()
                break

if __name__ == '__main__':
    bot = LichessBot(TOKEN_FILE_PATH, ENGINE_PATH, OPENING_BOOK_PATH)
    bot.MainLoop()