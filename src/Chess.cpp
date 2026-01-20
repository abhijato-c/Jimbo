#include "Defs.h"
#include "MovePiece.h"
#include "MoveGen.h"
#include "Misc.h"
#include "LookupTables.h"
#include "MakeMove.h"

int main(){
    GenerateLookupTables();
    
    string line, command;
    chess brd;

    cout << "Jimbo ready and accepting commands! Please use this in your chess GUI/Lichess bot, or see how to write UCI commands." << endl;
    while (getline(cin, line)) {
        stringstream ss(line);
        ss >> command;

        if (command == "uci") {cout << "uciok" << endl;} 
        else if (command == "isready") {cout << "readyok" << endl;} 
        else if (command == "position") {
            string sub;
            ss >> sub;
            if (sub == "startpos") {ParseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", brd);} 
            else if (sub == "fen") {
                string fenStr, part;
                while (ss >> part) fenStr += part + " ";
                ParseFEN(fenStr, brd);
            }
            
            // todo: moves suffix - position startpos moves e2e4
            string movesKeyword;
            if (ss >> movesKeyword && movesKeyword == "moves") {
                string moveStr;
                while (ss >> moveStr) {
                    //Make str -> move fucntion
                }
            }
        } 
        else if (command == "go") {
            int lim = 3000; // Default 3s
            string sub;
            Move best = 0;
            // todo: multiple constraints, wtime btime
            if(ss >> sub) {
                if (sub == "movetime") {
                    ss >> lim;
                    best = IterativeDeepening(brd, lim);
                }
                else if (sub == "depth") {
                    ss >> lim;
                    best = BestMove(brd, lim);
                }
            }
            else {
                best = IterativeDeepening(brd, lim);
            }
            cout << "bestmove ";
            cout << MoveToStr(best) << endl;
        } 
        //todo: stop command
        else if (command == "quit") { break; }
    }
    return 0;
}