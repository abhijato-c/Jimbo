#include "Defs.h"
#include "MovePiece.h"
#include "MoveGen.h"
#include "Misc.h"
#include "LookupTables.h"
#include "BestMove.h"
#include <thread>
#include <atomic>

std::atomic<bool> StopSignal(false);
std::thread SearchThread;

bool isConvertibleToInt(const string& str, int& value) {
    stringstream ss(str);
    ss >> value; 
    return ss && ss.eof(); 
}

void RunSearch(Board brd, int TimeLimit, int DepthLimit){
    Move best = 0;

    if (DepthLimit == 0) best = IterativeDeepening(brd, TimeLimit, INT_MAX);
    else if (TimeLimit == 0) best = IterativeDeepening(brd, INT64_MAX, DepthLimit);
    else  best = IterativeDeepening(brd, TimeLimit, DepthLimit);

    cout << "bestmove " << MoveToStr(best) << endl;
}

void StopSearch(){
    StopSignal.store(true);
    if (SearchThread.joinable()) SearchThread.join();
    StopSignal.store(false);
}

int main(){
    cout << "Generating lookup tables, hold tight..." << endl;
    GenerateLookupTables();

    string InitialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    string line;
    Board brd;
    ParseFEN(InitialFEN, brd);

    cout << "Jimbo ready and accepting commands!" << endl;
    cout << "Please use this binary in your chess GUI/Lichess bot, or see how to write UCI commands." << endl;

    while (getline(cin, line)) {
        // Split the command
        vector<string> command = {};
        istringstream stream(line);
        string token;
        while (stream >> token) {command.push_back(token);}
        if (command.empty()) continue;
        string cmd = command[0];

        if (cmd == "uci") {
            cout << "id name Jimbo" << endl;
            cout << "id author abhijato-c" << endl;
            cout << "uciok" << endl;
        } 
        else if (cmd == "isready") {cout << "readyok" << endl;} 
        else if (cmd == "position") {
            StopSearch();
            if (command.size() < 2) {
                cout << "info string position command needs arguments" << endl;
                continue;
            }

            string sub = command[1];
            int StartInd = -1;
            
            if (sub == "startpos") {
                ParseFEN(InitialFEN, brd);
                if (command.size() >= 3 && command[2] == "moves") StartInd = 3;
            }
            else if (sub == "fen") {
                size_t i;
                string fenStr = "";

                for (i = 2; i < command.size() && command[i] != "moves"; ++i) fenStr += command[i] + " ";
                ParseFEN(fenStr, brd);
                if (i < command.size() && command[i] == "moves") StartInd = i + 1;
            }
            else {
                cout << "info string Illegal subcommand " << sub << endl;
                continue;
            }
            
            for (size_t i = StartInd; StartInd >= 0 && i < command.size(); ++i) {
                Move m = StrToMove(command[i], brd);
                MovePiece(brd, m);
            }
        } 
        else if (cmd == "go") {
            StopSearch();

            int TimeLimit = 0;
            int DepthLimit = 0;
            int wtime = 0;
            int btime = 0;
            int winc = 0;
            int binc = 0;
            int moves = 30;
            bool inf = false;
            bool TimeSet = false;
            bool Success = true;

            for (size_t i = 1; i < command.size(); ++i) {
                string sub = command[i];
                int arg = 0;
                if (sub == "infinite") inf = true;
                else if (i + 1 < command.size() && isConvertibleToInt(command[i + 1], arg)){
                    if (sub == "movetime") {
                        TimeLimit = arg;
                        TimeSet = true;
                    }
                    else if (sub == "depth") DepthLimit = arg;
                    else if (sub == "wtime") wtime = arg;
                    else if (sub == "btime") btime = arg;
                    else if (sub == "winc") winc = arg;
                    else if (sub == "binc") binc = arg;
                    else if (sub == "movestogo") moves = arg;
                    else Success = false;
                    i++;
                }
                else Success = false;
            }

            if (inf) TimeLimit = INT_MAX;
            else if (!TimeSet && !(wtime == 0 || btime == 0)) {
                int time = (brd.turn == WHITE) ? wtime : btime;
                int inc = (brd.turn == WHITE) ? winc : binc;
                if (time > 0) {
                    TimeLimit = (time / moves) + (inc / 2);
                    if (TimeLimit < 10) TimeLimit = 10;
                    if (TimeLimit > time - 50 && time > 100) TimeLimit = time - 50;
                }
                else Success = false;
            }
            else if (!TimeSet && DepthLimit == 0) TimeLimit = INT_MAX;
            else if (DepthLimit < 0 || TimeLimit < 0 || wtime < 0 || btime < 0 || winc < 0 || binc < 0 || moves < 0) Success = false;

            if (Success)
                SearchThread = thread(RunSearch, brd, TimeLimit, DepthLimit);
            else
                cout << "info string invalid/malformed/unsupported go command" << endl;
        }
        else if (cmd == "ucinewgame") {
            StopSearch();
            ParseFEN(InitialFEN, brd);
            // Clear Transposition Tables
        }

        else if (cmd == "setoption") {
            // ignore
        }
        else if (cmd == "stop") {
            StopSearch();
        }
        else if (cmd == "quit") {
            StopSearch();
            break;
        }
        else {cout << "info string Illegal command " << cmd << "!" << endl;}
    }

    StopSearch();
    return 0;
}