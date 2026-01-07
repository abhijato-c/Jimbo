#include "Init.cpp"
#include "MoveGen.cpp"
#include "MovePiece.cpp"
#include "Misc.cpp"
#include "LookupTables.cpp"
#include <sstream>

int MiniMax(chess &b, int depth, int alpha, int beta, timept start = chrono::high_resolution_clock::now(), int time = 2147483647){
    if (depth <= 1) return StaticEval(b);
    
    //check for game over, higher eval for delayed checkmate
    if(b.wk==0){
        if (b.turn) return -128-depth;
        return 128+depth;
    }
    if (b.bk==0){
        if (b.turn) return 128+depth;
        return -128-depth;
    }

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
    int alpha = -999;
    int beta = 999;
    Move BestMove = 0;
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
            BestMove = Moves[MaxIndex];
        }
        if(alpha>=beta) break;
    }
    return BestMove;
}

Move IterativeDeepening(chess &b, int &t){
    auto s=chrono::high_resolution_clock::now();
    int d=2;
    Move bm=0;
    int besteval=-999;
    while((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count())<t){
        chess rd;
        int eval;
        int bestmove=0;
        int alpha=-999;
        int beta=999;
        vector<Move> mvs=pl_moves(b);

        //move ordering
        int advs[mvs.size()];
        for (int i=0;i<(int)mvs.size();++i){
            rd=b;
            move_piece(mvs[i],rd);
            advs[i]=-StaticEval(rd);
        }

        //evaluate advantage
        for(int i=0;i<(int)mvs.size();++i){
            //get the maximum advantage for highest ab efficiency
            int maxi=advs[0];
            int maxelem=0;
            for(int j=1;j<(int)mvs.size();++j){
                if(advs[j]>maxi){
                    maxi=advs[j];
                    maxelem=j;
                }
            }
            advs[maxelem]=-9999;
            rd=b;
            move_piece(mvs[maxelem],rd);
            eval = -MiniMax(rd, d, -beta, -alpha, s, t);
            if((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count())>t){
                if(alpha>besteval)
                    return bestmove; 
                return bm;
            }
            if(eval>alpha){
                alpha=eval;
                bestmove=mvs[maxelem];
            }
            if(alpha>=beta){break;}
        }
        besteval=alpha;
        bm=bestmove;
        d+=1;
    }
    return bm;
}

void profile(int iters=6, string fen="r1bq3r/pp1pkpN1/3p4/8/8/2Q4P/P3PP1P/R2K1B1R"){
    chess b;
    ParseFEN(fen,b);
    b.turn=false;
    auto start=chrono::high_resolution_clock::now();
    for(int i=1;i<=iters;++i){
        int best_move = BestMove(b, 6);
        move_piece(best_move,b);
        cout<<i<<"-";
        PrintMove(best_move);
    }
    cout<<endl;
    auto stop=chrono::high_resolution_clock::now();
    cout<<"time per move:"<<chrono::duration_cast<chrono::microseconds>(stop - start).count()/((double)1000000*iters)<<endl;
    cout<<"nps:"<<nodes*(double)1000000/chrono::duration_cast<chrono::microseconds>(stop - start).count()<<endl;
    cout<<"nodes per move:"<<nodes/(double)iters;
}

int main(){
    generate_rook_tables();
    generate_bishop_tables();
    
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
            if (sub == "startpos") {ParseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w", brd);} 
            else if (sub == "fen") {
                // todo: change to i<6 on full fen implementation
                string fenStr, part;
                for(int i=0; i<2; i++) { 
                    ss >> part; 
                    fenStr += part + " "; 
                }
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
            PrintMove(best);
        } 
        //todo: stop command
        else if (command == "quit") { break; }
    }
    return 0;
}