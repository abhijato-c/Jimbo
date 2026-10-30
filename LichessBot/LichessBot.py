import json
import logging
import logging.handlers
import os
import platform
import random
import signal
import sys
import threading
import time
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import chess
import chess.engine
import chess.polyglot
import requests
from requests.adapters import HTTPAdapter

BASE_URL = "https://lichess.org/"
CONSOLE_LEVEL = logging.INFO

# All of these are set from config.yaml by LoadConfig()
MAX_CONCURRENT_GAMES: int = 0
ACCEPT_RATED: bool = False
ACCEPT_CASUAL: bool = False
ACCEPTED_VARIANTS: set = set()
ACCEPTED_TIME_CONTROLS: set = set()
USE_MOVE_HISTORY: bool = False
MOVE_OVERHEAD: float = 0.0
MAX_MOVE_TIME: float = 0.0
MAX_GAME_RESTARTS = 10

VALID_VARIANTS = {"standard", "chess960", "crazyhouse", "antichess", "atomic", "horde",
                  "kingofthehill", "racingkings", "threecheck", "fromposition"}
VALID_TIME_CONTROLS = {"ultrabullet", "bullet", "blitz", "rapid", "classical", "correspondence", "unlimited"}
STREAM_READ_TIMEOUT = 30


BOT_DIR = Path("LichessBot")
TOKEN_FILE_PATH = BOT_DIR / "token.txt"
OPENING_BOOK_PATH = BOT_DIR / "OpeningBook.bin"
ENGINE_PATH = BOT_DIR / ("Engine.exe" if platform.system() == "Windows" else "Engine")
LOG_DIR = BOT_DIR / "logs"
GAME_LOG_DIR = LOG_DIR / "games"
STDERR_DIR = LOG_DIR / "engine_stderr"
CONFIG_FILE_PATH = BOT_DIR / "config.yaml"

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
    "listchlng": "api/challenge",
}

logger = logging.getLogger("bot")

class _ConsoleFilter(logging.Filter):
    """Console shows only the bot's own messages (no urllib3 / chess.engine chatter)."""
    def filter(self, record: logging.LogRecord) -> bool:
        return record.name == "bot" or record.name.startswith("bot.")

def SetupLogging():
    for d in (LOG_DIR, GAME_LOG_DIR, STDERR_DIR):
        d.mkdir(parents=True, exist_ok=True)

    detailed = logging.Formatter(
        "%(asctime)s.%(msecs)03d | %(levelname)-8s | %(threadName)-12s | %(name)s:%(funcName)s:%(lineno)d | %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )

    root = logging.getLogger()
    root.setLevel(logging.DEBUG)
    for h in list(root.handlers):
        root.removeHandler(h)

    console = logging.StreamHandler(sys.stdout)
    console.setLevel(CONSOLE_LEVEL)
    console.setFormatter(logging.Formatter("%(asctime)s %(levelname)-7s %(message)s", datefmt="%H:%M:%S"))
    console.addFilter(_ConsoleFilter())
    root.addHandler(console)

    main_file = logging.handlers.RotatingFileHandler(
        LOG_DIR / "bot.log", maxBytes=10_000_000, backupCount=10, encoding="utf-8")
    main_file.setLevel(logging.DEBUG)
    main_file.setFormatter(detailed)
    root.addHandler(main_file)

    # Raw engine conversation goes to its own file only
    engine_log = logging.getLogger("chess.engine")
    engine_log.setLevel(logging.DEBUG)
    engine_log.propagate = False
    eh = logging.handlers.RotatingFileHandler(
        LOG_DIR / "engine.log", maxBytes=20_000_000, backupCount=5, encoding="utf-8")
    eh.setFormatter(detailed)
    engine_log.addHandler(eh)

    logging.getLogger("urllib3").setLevel(logging.INFO)

    def _excepthook(exc_type, exc, tb):
        if issubclass(exc_type, KeyboardInterrupt):
            return
        logger.critical("Uncaught exception", exc_info=(exc_type, exc, tb))

    def _thread_hook(args):
        if args.exc_type is SystemExit:
            return
        logger.critical(f"Uncaught exception in thread {args.thread.name if args.thread else '?'}",
                        exc_info=(args.exc_type, args.exc_value, args.exc_traceback))

    sys.excepthook = _excepthook
    threading.excepthook = _thread_hook


class _GameLogAdapter(logging.LoggerAdapter):
    def process(self, msg, kwargs):
        return f"[{self.extra['gid']}] {msg}", kwargs


class FatalError(Exception):
    """Unrecoverable configuration problem (bad token, missing engine, ...)."""


def _AsStringSet(value: Any, key: str, valid: set) -> set:
    if isinstance(value, str):
        value = [value]
    if not isinstance(value, (list, tuple, set)) or not value:
        raise FatalError(f"Config '{key}' must be a non-empty list of strings.")
    result = {str(v).strip().lower() for v in value}
    bad = result - valid
    if bad:
        raise FatalError(f"Config '{key}' has invalid entries {sorted(bad)}; allowed: {sorted(valid)}")
    return result


def LoadConfig(path: Path):
    """Load all settings from a YAML file into the module-level settings. Every key is required."""
    global MAX_CONCURRENT_GAMES, ACCEPT_RATED, ACCEPT_CASUAL, ACCEPTED_VARIANTS
    global ACCEPTED_TIME_CONTROLS, USE_MOVE_HISTORY, MOVE_OVERHEAD, MAX_MOVE_TIME

    if not path.is_file():
        raise FatalError(f"Config file '{path}' not found.")
    try:
        import yaml
    except ImportError:
        raise FatalError("PyYAML is required to read config.yaml (pip install pyyaml).")
    try:
        cfg = yaml.safe_load(path.read_text(encoding="utf-8"))
    except Exception as e:
        raise FatalError(f"Could not parse '{path}': {e}")
    if not isinstance(cfg, dict):
        raise FatalError(f"Config '{path}' must contain a mapping of keys to values.")

    required = ["simultaneous_games", "accept_rated", "accept_casual", "accepted_variants",
                "accepted_time_controls", "use_move_history", "move_overhead", "max_move_time"]
    missing = [k for k in required if k not in cfg]
    if missing:
        raise FatalError(f"Config '{path}' is missing required keys: {', '.join(missing)}")

    def num(key, minimum, integer=False):
        v = cfg[key]
        if isinstance(v, bool) or not isinstance(v, (int, float)) or v < minimum:
            raise FatalError(f"Config '{key}' must be a number >= {minimum}, got {v!r}.")
        return int(v) if integer else float(v)

    def boolean(key):
        v = cfg[key]
        if not isinstance(v, bool):
            raise FatalError(f"Config '{key}' must be true or false, got {v!r}.")
        return v

    MAX_CONCURRENT_GAMES = num("simultaneous_games", 1, integer=True)
    ACCEPT_RATED = boolean("accept_rated")
    ACCEPT_CASUAL = boolean("accept_casual")
    ACCEPTED_VARIANTS = _AsStringSet(cfg["accepted_variants"], "accepted_variants", VALID_VARIANTS)
    ACCEPTED_TIME_CONTROLS = _AsStringSet(cfg["accepted_time_controls"], "accepted_time_controls",
                                          VALID_TIME_CONTROLS)
    USE_MOVE_HISTORY = boolean("use_move_history")
    MOVE_OVERHEAD = num("move_overhead", 0.0)
    MAX_MOVE_TIME = num("max_move_time", 0.05)

    unknown = set(cfg) - {"simultaneous_games", "accept_rated", "accept_casual", "accepted_variants",
                          "accepted_time_controls", "use_move_history", "move_overhead", "max_move_time"}
    if unknown:
        logger.warning(f"Ignoring unknown config keys: {', '.join(sorted(map(str, unknown)))}")
    logger.info(f"Config loaded: games={MAX_CONCURRENT_GAMES} rated={ACCEPT_RATED} casual={ACCEPT_CASUAL} "
                f"variants={sorted(ACCEPTED_VARIANTS)} time_controls={sorted(ACCEPTED_TIME_CONTROLS)} "
                f"history={USE_MOVE_HISTORY} overhead={MOVE_OVERHEAD}s max_move={MAX_MOVE_TIME}s")


class LichessAPIError(Exception):
    def __init__(self, status: int, body: str, url: str):
        super().__init__(f"HTTP {status} from {url}: {body}")
        self.status = status
        self.body = body
        self.url = url

_PIECE_VALUES = {chess.PAWN: 1, chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9, chess.KING: 0}

def FallbackMove(board: chess.Board) -> chess.Move:
    """Last-resort move if the engine is unusable: mate-in-1, then sensible captures, else random."""
    best, best_score = None, -1e9
    for m in board.legal_moves:
        attacker = board.piece_type_at(m.from_square)
        victim = chess.PAWN if board.is_en_passant(m) else board.piece_type_at(m.to_square)
        score = random.random() * 0.5
        if victim:
            score += _PIECE_VALUES[victim] * 10
        if m.promotion:
            score += _PIECE_VALUES[m.promotion] * 10
        if board.is_attacked_by(not board.turn, m.to_square):
            score -= _PIECE_VALUES.get(attacker, 0) * 9
        board.push(m)
        mate = board.is_checkmate()
        board.pop()
        if mate:
            return m
        if score > best_score:
            best, best_score = m, score
    if best is None:
        raise RuntimeError("No legal moves available for fallback")
    return best


def FormatScore(info: Optional[dict], color: chess.Color) -> str:
    try:
        score = (info or {}).get("score")
        if score is None:
            return ""
        p = score.pov(color)
        return f"#{p.mate()}" if p.is_mate() else f"{p.score() / 100:+.2f}"
    except Exception:
        return ""


def ToMs(value: Any, default: float) -> float:
    return float(value) if isinstance(value, (int, float)) else default

class EngineHandle:
    def __init__(self, bot: "LichessBot", path: Path, GameID: str, log: logging.LoggerAdapter):
        self.bot = bot
        self.path = path
        self.GameID = GameID
        self.log = log
        self.engine: Optional[chess.engine.SimpleEngine] = None
        self._stderr = None
        self.restarts = 0

    def Start(self):
        self.Close()
        STDERR_DIR.mkdir(parents=True, exist_ok=True)
        self._stderr = open(STDERR_DIR / f"{self.GameID}.log", "a", buffering=1, encoding="utf-8", errors="replace")
        self._stderr.write(f"\n===== engine start {time.strftime('%Y-%m-%d %H:%M:%S')} =====\n")
        try:
            self.engine = chess.engine.SimpleEngine.popen_uci(str(self.path), timeout=20, stderr=self._stderr)
        except Exception:
            self._CloseStderr()
            raise
        with self.bot.EnginesLock:
            self.bot.ActiveEngines.add(self)
        try:
            pid = self.engine.transport.get_pid()
        except Exception:
            pid = "?"
        self.log.debug(f"Engine started: pid={pid} id={getattr(self.engine, 'id', {})}")

    def _CloseStderr(self):
        try:
            if self._stderr:
                self._stderr.close()
        except Exception:
            pass
        self._stderr = None

    @staticmethod
    def _Kill(engine: chess.engine.SimpleEngine):
        try:
            engine.transport.kill()
        except Exception:
            pass

    def Close(self):
        eng, self.engine = self.engine, None
        with self.bot.EnginesLock:
            self.bot.ActiveEngines.discard(self)
        if eng is not None:
            def _shutdown():
                try:
                    eng.quit()
                except Exception:
                    try:
                        eng.close()
                    except Exception:
                        pass
            t = threading.Thread(target=_shutdown, daemon=True)
            t.start()
            t.join(5)
            if t.is_alive():
                self.log.warning("Engine did not quit in 5s; killing process")
            self._Kill(eng)
        self._CloseStderr()

    def Restart(self, reason: str):
        self.restarts += 1
        self.log.warning(f"Restarting engine (#{self.restarts}): {reason}")
        self.Close()
        self.Start()

    def _Watchdog(self, engine: chess.engine.SimpleEngine, hard: float):
        self.log.error(f"Engine exceeded hard deadline of {hard:.2f}s; killing it")
        self._Kill(engine)

    def Play(self, board: chess.Board, limit: float, hard: float) -> chess.engine.PlayResult:
        if self.engine is None:
            self.Start()
        engine = self.engine
        timer = threading.Timer(hard, self._Watchdog, args=(engine, hard))
        timer.daemon = True
        timer.start()
        try:
            return engine.play(
                board, chess.engine.Limit(time=limit),
                info=chess.engine.Info.BASIC | chess.engine.Info.SCORE)
        finally:
            timer.cancel()

class GameContext:
    def __init__(self, GameID: str, log: logging.LoggerAdapter, engine: EngineHandle):
        self.GameID = GameID
        self.log = log
        self.engine = engine
        self.color: Optional[chess.Color] = None
        self.initial_fen = "startpos"
        self.opponent = "?"
        self.last_sent_ply = -1
        self.finished = False
        self.announced = False
        self.book_ok = True
        self.engine_failures = 0
        self.fallback_moves = 0
        self.declined_takeback = False

class LichessBot:
    def __init__(self, TokenPath: Path, EnginePath: Path, OpeningBookPath: Path):
        self.stop_event = threading.Event()
        self._local = threading.local()
        self.token = self.LoadToken(TokenPath)
        self.EnginePath = EnginePath
        self.OpeningBookPath = OpeningBookPath
        self.OpeningBook: Optional[chess.polyglot.MemoryMappedReader] = None
        self.Username = ""

        self.ActiveEngines: set = set()
        self.EnginesLock = threading.Lock()
        self.Games: Dict[str, threading.Thread] = {}
        self.GamesLock = threading.Lock()

        self.InitResources()

    @staticmethod
    def LoadToken(path: Path) -> str:
        if not path.is_file():
            raise FatalError(f"Token file '{path}' not found.")
        token = path.read_text(encoding="utf-8").strip()
        if not token:
            raise FatalError(f"Token file '{path}' is empty.")
        return token

    def InitResources(self):
        if not self.EnginePath.is_file():
            raise FatalError(f"Engine file '{self.EnginePath}' not found.")
        if platform.system() != "Windows" and not os.access(self.EnginePath, os.X_OK):
            try:
                self.EnginePath.chmod(self.EnginePath.stat().st_mode | 0o111)
                logger.info("Engine file was not executable; fixed permissions.")
            except Exception as e:
                raise FatalError(f"Engine '{self.EnginePath}' is not executable and chmod failed: {e}")

        # Engine self-test
        try:
            logger.info("Testing engine...")
            eng = chess.engine.SimpleEngine.popen_uci(str(self.EnginePath), timeout=20)
            name = eng.id.get("name", "unknown")
            eng.quit()
            logger.info(f"Engine OK: {name}")
        except Exception as e:
            logger.debug("Engine self-test traceback", exc_info=True)
            raise FatalError(f"Engine failed its startup test: {e!r}")

        # Opening book
        if self.OpeningBookPath.is_file():
            try:
                self.OpeningBook = chess.polyglot.MemoryMappedReader(str(self.OpeningBookPath))
                logger.info("Opening book loaded.")
            except Exception as e:
                logger.warning(f"Could not load opening book: {e}")
        else:
            logger.warning("No opening book found. Running without one.")

        # Account
        profile = self._WaitForProfile()
        if profile.get("title") != "BOT":
            logger.info("Upgrading account to bot status...")
            try:
                self._request("POST", "upgrade")
            except LichessAPIError as e:
                raise FatalError(f"Could not upgrade account to BOT: {e}")
            profile = self._WaitForProfile()
        self.Username = str(profile.get("id", "")).lower()
        if not self.Username:
            raise FatalError("Could not determine username from profile.")
        logger.info(f"Bot '{self.Username}' is online and ready (max {MAX_CONCURRENT_GAMES} games).")

    def _WaitForProfile(self) -> dict:
        delay = 2.0
        while True:
            try:
                return self._request("GET", "profile").json()
            except FatalError:
                raise
            except (requests.RequestException, LichessAPIError) as e:
                logger.warning(f"Cannot reach Lichess ({e}); retrying in {delay:.0f}s")
                time.sleep(delay)
                delay = min(delay * 2, 60)

    def _session(self) -> requests.Session:
        s = getattr(self._local, "session", None)
        if s is None:
            s = requests.Session()
            s.headers.update({"Authorization": f"Bearer {self.token}", "User-Agent": "LichessBot-wrapper"})
            adapter = HTTPAdapter(pool_connections=4, pool_maxsize=4)
            s.mount("https://", adapter)
            s.mount("http://", adapter)
            self._local.session = s
        return s

    def _request(self, method: str, endpoint_key: str, *args, retries: int = 5, base_delay: float = 0.5,
                 max_delay: float = 30.0, timeout=(10, 30), **kwargs) -> requests.Response:
        """HTTP call with retry on network errors, 429 and 5xx. Other 4xx raise immediately."""
        url = BASE_URL + ENDPOINTS[endpoint_key].format(*args)
        last: Optional[Exception] = None
        for attempt in range(1, retries + 1):
            wait = min(max_delay, base_delay * 2 ** (attempt - 1)) + random.uniform(0, base_delay / 2)
            try:
                r = self._session().request(method, url, timeout=timeout, **kwargs)
            except requests.RequestException as e:
                last = e
            else:
                logger.debug(f"{method} {url} -> {r.status_code}")
                if r.status_code < 400:
                    return r
                err = LichessAPIError(r.status_code, r.text[:500], url)
                if r.status_code == 401:
                    raise FatalError(f"Lichess rejected the token (401): {err.body}")
                if r.status_code == 429:
                    try:
                        wait = float(r.headers.get("Retry-After", 60))
                    except ValueError:
                        wait = 60.0
                    logger.warning(f"Rate limited by Lichess; waiting {wait:.0f}s")
                elif r.status_code < 500:
                    raise err
                last = err
            if attempt < retries:
                logger.warning(f"{method} {endpoint_key} failed ({last}); retry {attempt}/{retries - 1} in {wait:.1f}s")
                time.sleep(wait)
        assert last is not None
        raise last

    def _Stream(self, endpoint_key: str, *args) -> requests.Response:
        url = BASE_URL + ENDPOINTS[endpoint_key].format(*args)
        r = self._session().get(url, stream=True, timeout=(10, STREAM_READ_TIMEOUT))
        if r.status_code != 200:
            body = r.text[:500]
            r.close()
            if r.status_code == 401:
                raise FatalError(f"Lichess rejected the token (401): {body}")
            raise LichessAPIError(r.status_code, body, url)
        return r

    def AcceptChallenge(self, id: str):
        self._request("POST", "accept", id)

    def RejectChallenge(self, id: str, reason: str = "generic"):
        self._request("POST", "decline", id, data={"reason": reason})

    def _DecideChallenge(self, ch: dict) -> Tuple[bool, str]:
        variant = str((ch.get("variant") or {}).get("key", "")).lower()
        if variant not in ACCEPTED_VARIANTS:
            return False, "standard" if ACCEPTED_VARIANTS == {"standard"} else "variant"
        tc_type = str((ch.get("timeControl") or {}).get("type", "")).lower()
        # Unlimited games have no speed category; everything else is bullet/blitz/rapid/etc.
        speed = "unlimited" if tc_type == "unlimited" else str(ch.get("speed", "")).lower()
        if speed not in ACCEPTED_TIME_CONTROLS:
            return False, "timeControl"
        if ch.get("rated") and not ACCEPT_RATED:
            return False, "casual"
        if not ch.get("rated") and not ACCEPT_CASUAL:
            return False, "rated"
        with self.GamesLock:
            if len(self.Games) >= MAX_CONCURRENT_GAMES:
                return False, "later"
        return True, ""

    def _OnChallenge(self, ch: dict):
        challenger = ch.get("challenger") or {}
        dest = ch.get("destUser") or {}
        if str(challenger.get("id", "")).lower() == self.Username:
            return  # our own outgoing challenge
        if dest.get("id") and str(dest["id"]).lower() != self.Username:
            return
        cid = ch["id"]
        tc = ch.get("timeControl") or {}
        desc = f"{ch.get('speed', '?')} {tc.get('show', '?')} {'rated' if ch.get('rated') else 'casual'}"
        logger.debug(f"Challenge payload: {json.dumps(ch)[:1500]}")
        accept, reason = self._DecideChallenge(ch)
        who = challenger.get("name") or challenger.get("id", "?")
        try:
            if accept:
                self.AcceptChallenge(cid)
                logger.info(f"Challenge {cid} from {who} ({desc}): accepted")
            else:
                self.RejectChallenge(cid, reason)
                logger.info(f"Challenge {cid} from {who} ({desc}): declined ({reason})")
        except (LichessAPIError, requests.RequestException) as e:
            logger.warning(f"Challenge {cid} response failed: {e}")

    def GetOngoingGames(self) -> List[Dict[str, Any]]:
        return self._request("GET", "playing").json().get("nowPlaying", [])

    def ResumeOngoingGames(self):
        try:
            games = self.GetOngoingGames()
        except Exception as e:
            logger.warning(f"Could not check for ongoing games: {e}")
            return
        for g in games:
            gid = g.get("gameId") or g.get("fullId", "")[:8]
            if gid:
                logger.info(f"Resuming ongoing game {gid}")
                self._StartGameThread(gid)

    def _HandleEvent(self, event: dict):
        etype = event.get("type")
        if etype == "challenge":
            self._OnChallenge(event["challenge"])
        elif etype == "gameStart":
            g = event.get("game") or {}
            gid = g.get("gameId") or g.get("id")
            if gid:
                self._StartGameThread(gid)
        elif etype == "gameFinish":
            g = event.get("game") or {}
            logger.debug(f"gameFinish event: {g.get('gameId') or g.get('id')}")
        else:
            logger.debug(f"Ignored event: {event}")

    def MainLoop(self):
        backoff = 1.0
        while not self.stop_event.is_set():
            try:
                with self._Stream("stream_event") as response:
                    logger.info("Connected to Lichess event stream.")
                    backoff = 1.0
                    for line in response.iter_lines():
                        if self.stop_event.is_set():
                            break
                        if not line:
                            continue
                        try:
                            event = json.loads(line.decode("utf-8"))
                        except ValueError:
                            logger.debug(f"Unparseable event line: {line[:200]!r}")
                            continue
                        try:
                            self._HandleEvent(event)
                        except FatalError:
                            raise
                        except Exception:
                            logger.exception(f"Error handling event {str(event)[:300]}")
                if not self.stop_event.is_set():
                    logger.warning("Event stream closed by server; reconnecting...")
            except (FatalError, KeyboardInterrupt):
                raise
            except (requests.RequestException, LichessAPIError) as e:
                logger.warning(f"Event stream error: {e}. Reconnecting in {backoff:.0f}s...")
                backoff = min(backoff * 2, 60.0)
            except Exception:
                logger.exception(f"Unexpected event-loop error. Reconnecting in {backoff:.0f}s...")
                backoff = min(backoff * 2, 60.0)
            self.stop_event.wait(backoff)

    def Run(self):
        try:
            self.ResumeOngoingGames()
            self.MainLoop()
        except KeyboardInterrupt:
            logger.info("Interrupted; shutting down.")
        finally:
            self.stop()

    def stop(self):
        self.stop_event.set()
        if self.OpeningBook:
            try:
                self.OpeningBook.close()
            except Exception:
                pass
            self.OpeningBook = None
        with self.EnginesLock:
            engines = list(self.ActiveEngines)
        for h in engines:
            try:
                h.Close()
            except Exception:
                pass
        logger.info("Bot shut down.")

    def _StartGameThread(self, GameID: str):
        with self.GamesLock:
            t = self.Games.get(GameID)
            if t is not None and t.is_alive():
                logger.debug(f"Game {GameID} already running; ignoring duplicate start")
                return
            t = threading.Thread(target=self.PlayGame, args=(GameID,), name=f"game-{GameID}", daemon=True)
            self.Games[GameID] = t
        t.start()

    def PlayGame(self, GameID: str):
        glog = _GameLogAdapter(logging.getLogger(f"bot.game.{GameID}"), {"gid": GameID})
        file_handler = None
        engine = None
        try:
            GAME_LOG_DIR.mkdir(parents=True, exist_ok=True)
            file_handler = logging.FileHandler(GAME_LOG_DIR / f"{GameID}.log", encoding="utf-8")
            file_handler.setLevel(logging.DEBUG)
            file_handler.setFormatter(logging.Formatter(
                "%(asctime)s.%(msecs)03d | %(levelname)-8s | %(funcName)s:%(lineno)d | %(message)s",
                datefmt="%H:%M:%S"))
            logging.getLogger(f"bot.game.{GameID}").addHandler(file_handler)

            engine = EngineHandle(self, self.EnginePath, GameID, glog)
            ctx = GameContext(GameID, glog, engine)
            try:
                engine.Start()
            except Exception as e:
                glog.warning(f"Engine failed to start ({e!r}); will retry on first move")
                glog.debug("Engine start traceback", exc_info=True)

            crashes = 0
            while not self.stop_event.is_set() and not ctx.finished:
                try:
                    self._PlayGame(ctx)
                    break
                except FatalError as e:
                    glog.critical(f"Fatal error: {e}")
                    break
                except Exception:
                    crashes += 1
                    glog.exception(f"Game loop crashed ({crashes}/{MAX_GAME_RESTARTS})")
                    if crashes >= MAX_GAME_RESTARTS:
                        glog.critical("Too many crashes; abandoning game")
                        break
                    self.stop_event.wait(min(2 * crashes, 10))
        except Exception:
            glog.exception("Game thread failed during setup")
        finally:
            if engine:
                try:
                    engine.Close()
                except Exception:
                    pass
            if file_handler:
                logging.getLogger(f"bot.game.{GameID}").removeHandler(file_handler)
                file_handler.close()
            with self.GamesLock:
                self.Games.pop(GameID, None)
            glog.debug("Game thread exited")

    def _IsGameOngoing(self, GameID: str) -> bool:
        try:
            return any((g.get("gameId") or g.get("fullId", "")[:8]) == GameID for g in self.GetOngoingGames())
        except Exception:
            return True  # can't tell; assume yes and keep trying

    def _PlayGame(self, ctx: GameContext):
        fails = 0
        while not self.stop_event.is_set() and not ctx.finished:
            try:
                with self._Stream("stream", ctx.GameID) as response:
                    fails = 0
                    for line in response.iter_lines():
                        if self.stop_event.is_set():
                            return
                        if not line:
                            continue
                        try:
                            event = json.loads(line.decode("utf-8"))
                        except ValueError:
                            ctx.log.debug(f"Unparseable game line: {line[:200]!r}")
                            continue
                        self._HandleGameEvent(ctx, event)
                        if ctx.finished:
                            return
                ctx.log.debug("Game stream closed by server")
            except LichessAPIError as e:
                if e.status == 404:
                    ctx.log.warning("Game not found on Lichess; stopping")
                    return
                ctx.log.warning(f"Game stream error: {e}")
            except requests.RequestException as e:
                ctx.log.warning(f"Game stream disconnected: {e}")
            if ctx.finished:
                return
            if not self._IsGameOngoing(ctx.GameID):
                ctx.log.info("Game is no longer in progress")
                return
            fails += 1
            self.stop_event.wait(min(fails, 10))

    def _HandleGameEvent(self, ctx: GameContext, event: dict):
        etype = event.get("type")
        if etype == "gameFull":
            white = event.get("white") or {}
            black = event.get("black") or {}
            if str(white.get("id", "")).lower() == self.Username:
                ctx.color, opp = chess.WHITE, black
            elif str(black.get("id", "")).lower() == self.Username:
                ctx.color, opp = chess.BLACK, white
            else:
                raise RuntimeError(f"Bot '{self.Username}' is neither white nor black in this game")
            ctx.initial_fen = event.get("initialFen", "startpos")
            ctx.opponent = opp.get("name") or opp.get("id") or "AI"
            if not ctx.announced:
                ctx.announced = True
                clock = event.get("clock") or {}
                tc = f"{clock.get('initial', 0) // 60000}+{clock.get('increment', 0) // 1000}" if clock else "no clock"
                rating = opp.get("rating")
                ctx.log.info(f"Started vs {ctx.opponent}{f' ({rating})' if rating else ''} | "
                             f"{'white' if ctx.color == chess.WHITE else 'black'} | {tc} | "
                             f"{'rated' if event.get('rated') else 'casual'}")
            ctx.log.debug(f"gameFull: {json.dumps(event)[:2000]}")
            state = event["state"]
        elif etype == "gameState":
            state = event
        else:
            ctx.log.debug(f"Game event ignored: {str(event)[:300]}")
            return

        if ctx.color is None:
            return

        status = state.get("status", "started")
        if status not in ("created", "started"):
            self._OnGameEnd(ctx, state, status)
            return

        # Build board
        moves = state.get("moves", "") or ""
        board = chess.Board() if ctx.initial_fen in ("startpos", "", None) else chess.Board(fen=ctx.initial_fen)
        for mv in moves.split():
            board.push_uci(mv)

        # Decline takeback offers (best effort)
        opp_takeback = state.get("btakeback" if ctx.color == chess.WHITE else "wtakeback")
        if opp_takeback and not ctx.declined_takeback:
            ctx.declined_takeback = True
            try:
                self._request("POST", "takeback", ctx.GameID, "no", retries=2)
                ctx.log.info("Declined takeback offer")
            except Exception as e:
                ctx.log.debug(f"Takeback decline failed: {e}")
        elif not opp_takeback:
            ctx.declined_takeback = False

        if board.turn != ctx.color or board.is_game_over():
            return
        ply = len(board.move_stack)
        if ply == ctx.last_sent_ply:
            ctx.log.debug(f"Already sent a move for ply {ply}; skipping duplicate state")
            return

        # Clock
        left_ms = ToMs(state.get("wtime" if ctx.color == chess.WHITE else "btime"), 60_000)
        inc_ms = ToMs(state.get("winc" if ctx.color == chess.WHITE else "binc"), 0)
        left = left_ms / 1000
        limit = self.CalculateTimeLimit(left, board.fullmove_number, inc_ms / 1000)
        hard = min(limit + max(1.0, limit * 0.5), max(limit + 0.1, left - MOVE_OVERHEAD))
        ctx.log.debug(f"Our turn ply={ply} fen={board.fen()} left={left:.2f}s inc={inc_ms / 1000:.2f}s "
                      f"limit={limit:.3f}s hard={hard:.3f}s")

        t0 = time.monotonic()
        move, source, extra = self.SelectMove(ctx, board, limit, hard, left)
        elapsed = time.monotonic() - t0

        san = board.san(move)
        prefix = f"{board.fullmove_number}{'.' if ctx.color == chess.WHITE else '...'}"
        if self._SendMove(ctx, move.uci()):
            ctx.last_sent_ply = ply
            ctx.log.info(f"{prefix} {san:<8} {source}, {elapsed:.2f}s{(' ' + extra) if extra else ''}")

    def _SendMove(self, ctx: GameContext, uci: str) -> bool:
        try:
            self._request("POST", "move", ctx.GameID, uci, retries=8, base_delay=0.2, max_delay=2.0, timeout=(5, 10))
            return True
        except LichessAPIError as e:
            ctx.log.warning(f"Move {uci} rejected: {e.body}")
        except requests.RequestException as e:
            ctx.log.error(f"Move {uci} could not be sent: {e}")
        return False

    def _OnGameEnd(self, ctx: GameContext, state: dict, status: str):
        ctx.finished = True
        winner = state.get("winner")
        if winner is None:
            result = "DRAW" if status in ("draw", "stalemate") else "NO RESULT"
        else:
            result = "WIN" if (winner == "white") == (ctx.color == chess.WHITE) else "LOSS"
        ctx.log.info(f"Game over: {result} ({status}) vs {ctx.opponent}")
        ctx.log.debug(f"Final state: {state}")
        if ctx.engine_failures or ctx.fallback_moves:
            ctx.log.warning(f"Engine problems this game: failures={ctx.engine_failures}, "
                            f"fallback moves={ctx.fallback_moves}, restarts={ctx.engine.restarts}")
        try:
            b = chess.Board() if ctx.initial_fen in ("startpos", "", None) else chess.Board(fen=ctx.initial_fen)
            for mv in (state.get("moves") or "").split():
                b.push_uci(mv)
            ctx.log.debug(f"Moves: {chess.Board(b.root().fen()).variation_san(b.move_stack)}")
        except Exception:
            pass

    # ---- move selection ----------------------------------------------------------
    def CalculateTimeLimit(self, SecondsLeft: float, MovesPlayed: int, Increment: float = 0.0) -> float:
        MovesLeft = max(60 - MovesPlayed, 15)
        Time = SecondsLeft / MovesLeft + Increment * 0.75 - MOVE_OVERHEAD
        Time = min(Time, SecondsLeft * 0.3, MAX_MOVE_TIME)
        return max(0.02, Time)

    def SelectMove(self, ctx: GameContext, board: chess.Board, limit: float, hard: float,
                   left: float) -> Tuple[chess.Move, str, str]:
        # Opening book
        if self.OpeningBook and ctx.book_ok:
            try:
                entry = self.OpeningBook.weighted_choice(board)
                if entry.move in board.legal_moves:
                    return entry.move, "book", ""
            except IndexError:
                pass
            except Exception:
                ctx.log.exception("Opening book error; disabling book for this game")
                ctx.book_ok = False

        # Engine (attempt 2 restarts the engine and sends a bare FEN position)
        start = time.monotonic()
        for attempt in (1, 2):
            used = time.monotonic() - start
            if attempt == 2 and left - used < 1.0:
                ctx.log.warning("Too little time left for an engine retry")
                break
            lim = limit if attempt == 1 else max(0.05, limit - used)
            hd = hard if attempt == 1 else lim + max(1.0, lim * 0.5)
            try:
                if attempt == 2:
                    ctx.engine.Restart("previous attempt failed")
                pos = board if (USE_MOVE_HISTORY and attempt == 1) else chess.Board(board.fen())
                result = ctx.engine.Play(pos, lim, hd)
                move = result.move
                if move is None or move not in board.legal_moves:
                    raise chess.engine.EngineError(f"Engine returned unusable move: {move}")
                info = result.info or {}
                ctx.log.debug(f"Engine result: move={move.uci()} info={info}")
                extra = " ".join(x for x in (
                    FormatScore(info, ctx.color),
                    f"d{info['depth']}" if "depth" in info else "") if x)
                return move, "engine", extra
            except Exception as e:
                ctx.engine_failures += 1
                ctx.log.warning(f"Engine error on attempt {attempt}: {e!r} ({type(e).__name__})")
                ctx.log.debug("Engine error traceback", exc_info=True)
                ctx.log.error(f"Position: root={board.root().fen()} moves="
                              f"{' '.join(m.uci() for m in board.move_stack)} limit={lim:.3f}s")
                try:
                    if ctx.engine.engine is None:
                        ctx.log.debug("Engine handle empty after failure")
                    else:
                        ctx.log.debug(f"Engine returncode: {ctx.engine.engine.protocol.returncode}")
                except Exception:
                    pass

        # Fallback so we never lose on time / forfeit
        ctx.fallback_moves += 1
        move = FallbackMove(board)
        ctx.log.error(f"Using FALLBACK move {move.uci()} (engine unavailable)")
        try:  # leave a fresh engine ready for the next move
            ctx.engine.Restart("after fallback")
        except Exception as e:
            ctx.log.error(f"Engine restart failed: {e!r}")
        return move, "FALLBACK", ""


def _Main() -> int:
    SetupLogging()
    try:
        signal.signal(signal.SIGTERM, lambda *_: (_ for _ in ()).throw(KeyboardInterrupt()))
    except Exception:
        pass
    try:
        LoadConfig(CONFIG_FILE_PATH)
        bot = LichessBot(TOKEN_FILE_PATH, ENGINE_PATH, OPENING_BOOK_PATH)
        bot.Run()
        return 0
    except FatalError as e:
        logger.critical(str(e))
        return 1
    except KeyboardInterrupt:
        return 0
    except Exception:
        logger.critical("Unrecoverable error", exc_info=True)
        return 1


if __name__ == "__main__":
    sys.exit(_Main())