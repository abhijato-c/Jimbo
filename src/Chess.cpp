#include "Defs.h"
#include "MovePiece.h"
#include "MoveGen.h"
#include "Misc.h"
#include "LookupTables.h"
#include "MakeMove.h"

bool isConvertibleToInt(const std::string& str, int& value) {
    std::stringstream ss(str);
    ss >> value; 
    return ss && ss.eof(); 
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
            else { cout << "Illegal command!" << endl; }
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

            if (TimeLimit == 0 && DepthLimit == 0){
                best = IterativeDeepening(brd, 3000);
            }
            else if (DepthLimit == 0){
                best = IterativeDeepening(brd, TimeLimit);
            }
            else if (TimeLimit == 0){
                best = BestMove(brd, DepthLimit);
            }
            else {
                best = IterativeDeepening(brd, TimeLimit, DepthLimit);
            }

            cout << "bestmove " << MoveToStr(best) << endl;
        }
        else if (cmd == "ucinewgame") {
            // Clear search history/Transposition Tables, dont exist now so ignore to prevent illegal cmd
        }

        else if (cmd == "setoption") {
            // ignore
        }
        else if (cmd == "quit") { break; }
        else { cout << "Illegal command!" << endl;}
    }
    return 0;
}