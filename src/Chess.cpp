#include "Defs.h"
#include "MovePiece.h"
#include "MoveGen.h"
#include "Misc.h"
#include "LookupTables.h"
#include "MakeMove.h"
#include <thread>
#include <atomic>

std::atomic<bool> StopSignal(false);
std::thread SearchThread;

bool isConvertibleToInt(const string& str, int& value) {
    stringstream ss(str);
    ss >> value; 
    return ss && ss.eof(); 
}

void RunSearch(chess brd, int TimeLimit, int DepthLimit){
    StopSignal = false;

    Move best = 0;

    if (TimeLimit == 0 && DepthLimit == 0) best = IterativeDeepening(brd, 3000, INT_MAX);
    else if (DepthLimit == 0) best = IterativeDeepening(brd, TimeLimit, INT_MAX);
    else if (TimeLimit == 0) best = IterativeDeepening(brd, INT64_MAX, DepthLimit);
    else  best = IterativeDeepening(brd, TimeLimit, DepthLimit);

    cout << "bestmove " << MoveToStr(best) << endl;
}

void StopSearch(){
    StopSignal = true;
    if (SearchThread.joinable()) SearchThread.join();
}

int main(){
    cout << "Generating lookup tables, hold tight..." << endl;
    GenerateLookupTables();

    string line;
    chess brd;
    ParseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", brd);

    cout << "Jimbo ready and accepting commands!" << endl;
    cout << "Please use this binary in your chess GUI/Lichess bot, or see how to write UCI commands." << endl;

    while (getline(cin, line)) {
        StopSearch();

        // Split the command
        vector<string> command = {};
        istringstream stream(line);
        string token;
        while (getline(stream, token, ' ')) {command.push_back(token);}
        string cmd = command[0];

        if (cmd == "uci") {cout << "uciok" << endl;} 
        else if (cmd == "isready") {cout << "readyok" << endl;} 
        else if (cmd == "position") {
            string sub = command[1];
            int StartInd = -1;
            
            if (sub == "startpos") {
                ParseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", brd);
                for (int i = 2; i < (int)command.size(); ++i){
                    if (command[i] == "moves") {
                        StartInd = i+1;
                        break;
                    }
                }
            }
            else if (sub == "fen") {
                int i = 2;
                string fenStr = "";

                for (; i < (int)command.size() && command[i] != "moves"; ++i) fenStr += command[i] + " ";
                ParseFEN(fenStr, brd);
                if (i < (int)command.size() && command[i] == "moves")
                    StartInd = i + 1;
            }
            else {
                cout << "Illegal subcommand " << sub << "!" << endl;
                continue;
            }
            
            for (int i = StartInd; StartInd >= 0 && i < (int)command.size(); ++i) {
                Move m = StrToMove(command[i], brd);
                MovePiece(m, brd);
            }
        } 
        else if (cmd == "go") {
            string sub;
            int arg;
            bool Success = true;

            int TimeLimit = 0;
            int DepthLimit = 0;

            for(int i = 1; i < (int)command.size(); i += 2){
                sub = command[i]; 
                if (sub != "movetime" && sub != "depth"){
                    cout << "Illegal/unsupported subcommand " << sub << "!" << endl;
                    Success = false;
                    break;
                }
                if ((int)command.size() == i+1) {
                    cout << "Incomplete go command!" << endl;
                    Success = false;
                    break;
                }
                if (!isConvertibleToInt(command[i+1], arg)){
                    cout << "Invalid argument to " << sub << ": " << command[i+1] << endl;
                    Success = false;
                    break;
                }

                if (sub == "movetime"){
                    TimeLimit = arg;
                }
                else if (sub == "depth") {
                    DepthLimit = arg;
                }
            }

            if (!Success) continue;
            SearchThread = thread(RunSearch, brd, TimeLimit, DepthLimit);
        }
        else if (cmd == "ucinewgame") {
            // Clear Transposition Tables
        }

        else if (cmd == "setoption") {
            // ignore
        }
        else if (cmd == "stop") {
            // search already stopped when input arrived, ignore.
        }
        else if (cmd == "quit") { break; }
        else { cout << "Illegal command " << cmd << "!" << endl;}
    }
    return 0;
}