#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Defs.h"
#include "MoveGen.h"
#include "MovePiece.h"
#include "Misc.h"

// TODO - EARLY CAPTURE INCENTIVE (PROMOTION)

inline int MiniMax(chess &b, int depth, int alpha, int beta, timept start = chrono::high_resolution_clock::now(), int time = 2147483647){
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


inline Move BestMove(chess &b, int MaxDepth){
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

inline Move IterativeDeepening(chess &b, int &t){
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

inline void profile(int iters=6, string fen="r1k4r/2p1bq2/b4n1p/pp4p1/3QP3/7N/PP3PPP/RNB1R2K w - - 1 19"){
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

#endif