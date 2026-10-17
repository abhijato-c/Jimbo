#include "Defs.h"
#include "MovePiece.h"
#include "MoveGen.h"
#include "Misc.h"
#include "LookupTables.h"
#include "MakeMove.h"

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
            string sub = (command.size() > 1) ? command[1] : "";

            if (sub == "") best = IterativeDeepening(brd, 3000);
            else if (sub == "movetime") best = IterativeDeepening(brd, stoi(command[2]));
            else if (sub == "depth") best = BestMove(brd, stoi(command[2]));
            else cout << "Illegal command!" << endl;

            if (best != 0) cout << "bestmove " << MoveToStr(best) << endl;
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