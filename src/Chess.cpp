#include "Init.cpp"
#include "MoveGen.cpp"
#include "MovePiece.cpp"
#include "Misc.cpp"
#include "LookupTables.cpp"
#include <sstream>

// TODO - EARLY CAPTURE INCENTIVE (PROMOTION)

int MiniMax(chess &b, int depth, int alpha, int beta, timept start = chrono::high_resolution_clock::now(), int time = 2147483647){
    if(depth == 1) return StaticEval(b);
    //check for game over, higher eval for delayed checkmate
    if(b.wk == 0 || b.bk == 0) return (-128 - depth);

    chess rd;
    vector<Move> Moves = pl_moves(b);

    // Compute static evals of each position for move ordering
    int StaticEvals[Moves.size()];
    for (int i=0; i<(int)Moves.size(); ++i){
        rd=b;
        move_piece(Moves[i], rd);
        StaticEvals[i] = -StaticEval(rd);
    }

    // Evaluate positions in order of static evals
    for(int i=0; i<(int)Moves.size(); ++i){
        // Find position with highest static eval
        int MaxEval = StaticEvals[0];
        int MaxIndex = 0;
        for(int j=1; j<(int)Moves.size(); ++j){
            if(StaticEvals[j] > MaxEval){ 
                MaxEval = StaticEvals[j]; 
                MaxIndex=j;
            }
        }
        StaticEvals[MaxIndex] = -inf;
        rd=b;
        move_piece(Moves[MaxIndex], rd);
        alpha = max(-MiniMax(rd, depth-1, -beta, -alpha, start, time), alpha);
        if(alpha>=beta) return alpha;

        if((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-start).count())>time) break;
    }
    return alpha;
}


Move BestMove(chess &b, int MaxDepth){
    chess rd;
    int eval;
    int alpha = -inf;
    int beta = inf;
    Move Best = 0;
    vector<Move> Moves = pl_moves(b);

    // Compute static evals of each position for move ordering
    int StaticEvals[Moves.size()];
    for (int i=0; i<(int)Moves.size(); ++i){
        rd=b;
        move_piece(Moves[i], rd);
        StaticEvals[i] = -StaticEval(rd);
    }

    // Evaluate positions in order of static evals
    for(int i=0; i<(int)Moves.size(); ++i){
        // Find position with highest static eval
        int MaxEval = StaticEvals[0];
        int MaxIndex = 0;
        for(int j=1; j<(int)Moves.size(); ++j){
            if(StaticEvals[j] > MaxEval){ 
                MaxEval = StaticEvals[j]; 
                MaxIndex=j;
            }
        }
        StaticEvals[MaxIndex] = -inf;
        rd=b;
        move_piece(Moves[MaxIndex], rd);
        eval = -MiniMax(rd, MaxDepth, -beta, -alpha);
        if(eval>alpha){
            alpha = eval;
            Best = Moves[MaxIndex];
        }
        if(alpha>=beta) break;
    }
    return Best;
}

Move IterativeDeepening(chess &b, int &t){
    auto s=chrono::high_resolution_clock::now();
    int d=2;
    Move bm=0;
    int besteval = -inf;
    while((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count())<t){
        chess rd;
        int eval;
        int bestmove = 0;
        int alpha = -inf;
        int beta = inf;
        vector<Move> Moves = pl_moves(b);

        // Compute static evals of each position for move ordering
        int StaticEvals[Moves.size()];
        for (int i=0; i<(int)Moves.size(); ++i){
            rd=b;
            move_piece(Moves[i], rd);
            StaticEvals[i] = -StaticEval(rd);
        }

        // Evaluate positions in order of static evals
        for(int i=0; i<(int)Moves.size(); ++i){
            // Find position with highest static eval
            int MaxEval = StaticEvals[0];
            int MaxIndex = 0;
            for(int j=1; j<(int)Moves.size(); ++j){
                if(StaticEvals[j] > MaxEval){ 
                    MaxEval = StaticEvals[j]; 
                    MaxIndex=j;
                }
            }
            StaticEvals[MaxIndex] = -inf;
            rd=b;
            move_piece(Moves[MaxIndex], rd);
            eval = -MiniMax(rd, d, -beta, -alpha, s, t);
            if((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count())>t){
                if(alpha>besteval) return bestmove; 
                return bm;
            }
            if(eval>alpha){
                alpha=eval;
                bestmove=Moves[MaxIndex];
            }
            if(alpha>=beta) break;
        }
        besteval=alpha;
        bm=bestmove;
        d+=1;
    }
    return bm;
}

void profile(int iters=6, string fen="r1k4r/2p1bq2/b4n1p/pp4p1/3QP3/7N/PP3PPP/RNB1R2K w - - 1 19"){
    chess b;
    ParseFEN(fen,b);
    b.turn=false;
    auto start=chrono::high_resolution_clock::now();
    for(int i=1;i<=iters;++i){
        int best_move = BestMove(b, 7);
        move_piece(best_move,b);
        cout<<i<<"-";
        cout << MoveToStr(best_move) << endl;
    }
    cout<<endl;
    auto stop=chrono::high_resolution_clock::now();
    cout<<"time per move:"<<chrono::duration_cast<chrono::microseconds>(stop - start).count()/((double)1000000*iters)<<endl;
    cout<<"nps:"<<nodes*(double)1000000/chrono::duration_cast<chrono::microseconds>(stop - start).count()<<endl;
    cout<<"nodes per move:"<<nodes/(double)iters<<endl;
}

int main(){
    GenerateLookupTables();
    
    string line, command;
    chess brd;

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