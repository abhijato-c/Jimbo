#include "Defs.h"
#include "MovePiece.h"
#include "MoveGen.h"
#include "Misc.h"
#include "LookupTables.h"
#include "MakeMove.h"
#include <thread>
#include <atomic>

std::atomic<bool> StopSignal(false);
std::atomic<bool> IsSearching(false);
std::thread SearchThread;

bool isConvertibleToInt(const string& str, int& value) {
    stringstream ss(str);
    ss >> value; 
    return ss && ss.eof(); 
}

void RunSearch(chess brd, int TimeLimit, int DepthLimit){
    IsSearching = true;
    StopSignal = false;

    Move best = 0;

    if (TimeLimit == 0 && DepthLimit == 0) best = IterativeDeepening(brd, 3000, INT_MAX);
    else if (DepthLimit == 0) best = IterativeDeepening(brd, TimeLimit, INT_MAX);
    else if (TimeLimit == 0) best = IterativeDeepening(brd, INT64_MAX, DepthLimit);
    else  best = IterativeDeepening(brd, TimeLimit, DepthLimit);

    cout << "bestmove " << MoveToStr(best) << endl;
    IsSearching = false;
}

void StopSearch(){
    StopSignal = true;
    if (SearchThread.joinable()) SearchThread.join();
}

int main(){
    cout << "Generating lookup tables, hold tight..." << endl;
    GenerateLookupTables();
    
    string line, command;
    chess brd;

    cout << "Jimbo ready and accepting commands!" << endl;
    cout << "Please use this binary in your chess GUI/Lichess bot, or see how to write UCI commands." << endl;

    while (getline(cin, line)) {
        // Split the command
        vector<string> command = {};
        istringstream stream(line);
        string token;
        while (getline(stream, token, ' ')) {
            command.push_back(token);
        }

        string cmd = command[0];
        StopSearch();

        if (cmd == "uci") {cout << "uciok" << endl;} 
        else if (cmd == "isready") {cout << "readyok" << endl;} 
        else if (cmd == "position") {
            string sub = command[1];

            if (sub == "startpos") {ParseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", brd);} 
            else if (sub == "fen") {
                string fenStr = "";
                for (int i = 2; i < (int)command.size(); ++i){fenStr += command[i] + " ";}
                ParseFEN(fenStr, brd);
            }
            else if (sub == "moves"){
                cout << "Moves command currently not supported, please try fen or startpos!" << endl;
            }
            else { cout << "Illegal subcommand " << sub << "!" << endl; }
        } 
        else if (cmd == "go") {
            Move best = 0;
            string sub;
            int arg;
            bool Success = true;

            int TimeLimit = 0;
            int DepthLimit = 0;

            for(int i = 1; i < command.size(); i += 2){
                sub = command[i]; 
                if (sub != "movetime" && sub != "depth"){
                    cout << "Illegal/unsupported subcommand " << sub << "!" << endl;
                    Success = false;
                    break;
                }
                if (command.size() == i+1) {
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
            // Clear search history/Transposition Tables, dont exist now so ignore to prevent illegal cmd
        }

        else if (cmd == "setoption") {
            // ignore
        }
        else if (cmd == "stop") {
            // search already stopped when input arrived, can ignore.
        }
        else if (cmd == "quit") { break; }
        else { cout << "Illegal command " << cmd << "!" << endl;}
    }
    return 0;
}