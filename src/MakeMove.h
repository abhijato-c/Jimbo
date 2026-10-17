#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Defs.h"
#include "MoveGen.h"
#include "MovePiece.h"
#include "Misc.h"

// TODO - EARLY CAPTURE INCENTIVE (PROMOTION)

inline int MiniMax(const chess &b, int depth, int alpha, int beta, timept start = chrono::high_resolution_clock::now(), int64_t time = INT64_MAX){
    if(depth == 1) return StaticEval(b);
    //check for game over, higher eval for delayed checkmate
    if(b.wk == 0 || b.bk == 0) return (-128 - depth);

    chess rd;
    MoveList Moves = PseudoLegals(b);

    // Compute static evals of each position for move ordering
    int StaticEvals[Moves.size()];
    for (int i=0; i<Moves.size(); ++i){
        Move m = Moves[i];
        int score = 0;
    
        // Capture check
        if (b.pieces & (1ULL << ((m >> 6) & 63))) {
            score += 100;
        }
        // Promotions
        score += ((m >> 12) & 7) * 100;

        StaticEvals[i] = score;
    }

    // Evaluate positions in order of static evals
    for(int i=0; i<Moves.size(); ++i){
        // Find position with highest static eval
        int MaxEval = StaticEvals[0];
        int MaxIndex = 0;
        for(int j=1; j<Moves.size(); ++j){
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
        if(TimeElapsed(start) > time) break;
    }
    return alpha;
}


inline Move BestMove(const chess &b, int MaxDepth){
    chess rd;
    int eval;
    int alpha = -inf;
    int beta = inf;
    Move Best = 0;
    MoveList Moves = PseudoLegals(b);

    // Compute static evals of each position for move ordering
    int StaticEvals[Moves.size()];
    for (int i=0; i<Moves.size(); ++i){
        Move m = Moves[i];
        int score = 0;
    
        // Capture check
        if (b.pieces & (1ULL << ((m >> 6) & 63))) {
            score += 100;
        }
        // Promotions
        score += ((m >> 12) & 7) * 100;

        StaticEvals[i] = score;
    }

    // Evaluate positions in order of static evals
    for(int i=0; i<Moves.size(); ++i){
        // Find position with highest static eval
        int MaxEval = StaticEvals[0];
        int MaxIndex = 0;
        for(int j=1; j<Moves.size(); ++j){
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

inline Move IterativeDeepening(const chess &b, int64_t t, int Mdepth = INT32_MAX){
    timept StartTime = chrono::high_resolution_clock::now();
    int CurrentDepth=2;
    Move OverallBestMove = 0;
    int BestEval = -inf;
    while(TimeElapsed(StartTime) < t && CurrentDepth <= Mdepth){
        chess rd;
        int eval;
        int bestmove = 0;
        int alpha = -inf;
        int beta = inf;
        MoveList Moves = PseudoLegals(b);

        // Compute static evals of each position for move ordering
        int StaticEvals[Moves.size()];
        for (int i=0; i<Moves.size(); ++i){
            Move m = Moves[i];
            int score = 0;
        
            // Capture check
            if (b.pieces & (1ULL << ((m >> 6) & 63))) {
                score += 100;
            }
            // Promotions
            score += ((m >> 12) & 7) * 100;

            StaticEvals[i] = score;
        }

        // Evaluate positions in order of static evals
        for(int i=0; i<Moves.size(); ++i){
            // Find position with highest static eval
            int MaxEval = StaticEvals[0];
            int MaxIndex = 0;
            for(int j=1; j<Moves.size(); ++j){
                if(StaticEvals[j] > MaxEval){ 
                    MaxEval = StaticEvals[j]; 
                    MaxIndex=j;
                }
            }
            StaticEvals[MaxIndex] = -inf;
            rd=b;
            move_piece(Moves[MaxIndex], rd);
            eval = -MiniMax(rd, CurrentDepth, -beta, -alpha, StartTime, t);
            if(TimeElapsed(StartTime) > t){
                if(alpha > BestEval) return bestmove; 
                return OverallBestMove;
            }
            if(eval>alpha){
                alpha=eval;
                bestmove=Moves[MaxIndex];
            }
            if(alpha>=beta) break;
        }
        BestEval = alpha;
        OverallBestMove = bestmove;
        cout << "Depth: " << CurrentDepth << ", Move: " << MoveToStr(OverallBestMove) << endl;
        ++CurrentDepth;
    }
    return OverallBestMove;
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
    int64_t TimeTaken = TimeElapsed(start);
    cout<<endl;
    cout << "time per move:" <<  TimeTaken / ((double)1000000*iters) << endl;
    cout<<"nps:" << nodes*(double)1000000/TimeTaken << endl;
    cout<<"nodes per move:"<<nodes/(double)iters<<endl;
}

#endif