#include "Init.cpp"
#include "MoveGen.cpp"
#include "MovePiece.cpp"
#include "Misc.cpp"
#include "LookupTables.cpp"

int MiniMax(chess &b, int depth, int alpha, int beta, timept start = chrono::high_resolution_clock::now(), int time = 2147483647){
    if (depth <= 1) return seval(b);
    
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
        StaticEvals[i] = -seval(rd);
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
        StaticEvals[i] = -seval(rd);
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

Move iterative_deepening(chess &b, int &t){
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
            advs[i]=-seval(rd);
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
        prtmv(best_move);
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
    
    int tim=3000;
    chess brd;
    string fen;
    char t;
    while(true){
        cin>>fen;
        cin>>t;
        if (t=='w')
            brd.turn=true;
        else
            brd.turn=false;
        
        ParseFEN(fen,brd);
        //prtmv(BestMove(brd, 6));
        prtmv(iterative_deepening(brd,tim));
    }
    return 0;
}