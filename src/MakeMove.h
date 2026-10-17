#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Defs.h"
#include "MoveGen.h"
#include "MovePiece.h"
#include "Misc.h"

// TODO - EARLY CAPTURE INCENTIVE (PROMOTION)

inline int MiniMax(chess &b, int depth, int alpha, int beta, timept start = chrono::high_resolution_clock::now(), int64_t time = INT64_MAX){
    //check for game over, higher eval for delayed checkmate
    if(b.wk == 0 || b.bk == 0) return (-inf + 30 - depth);
    if(depth == 1) return StaticEval(b);

    MoveList Moves = PseudoLegals(b);

    // Compute static evals of each position for move ordering
    int StaticEvals[Moves.size()];
    for (int i=0; i<Moves.size(); ++i){
        Move m = Moves[i];
        int score = 0;
    
        // Capture check
        if ((b.pieces & (1ULL << ((m >> 6) & 63))) || ((m >> 18) & 7) == 7) {
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
        MovePiece(Moves[MaxIndex], b);
        alpha = max(-MiniMax(b, depth-1, -beta, -alpha, start, time), alpha);
        UnmovePiece(Moves[MaxIndex], b);

        if(alpha >= beta || TimeElapsed(start) > time || StopSignal.load()) return alpha;
    }
    return alpha;
}

inline Move IterativeDeepening(chess &b, int64_t time, int Mdepth){
    timept StartTime = chrono::high_resolution_clock::now();
    int CurrentDepth = 2;
    Move OverallBestMove = 0;
    int BestEval = -inf;
    while(TimeElapsed(StartTime) < time && CurrentDepth <= Mdepth && !StopSignal.load() && !(abs(BestEval) >= 300 && CurrentDepth > 2)){
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
            if ((b.pieces & (1ULL << ((m >> 6) & 63))) || ((m >> 18) & 7) == 7) {
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
            MovePiece(Moves[MaxIndex], b);
            eval = -MiniMax(b, CurrentDepth, -beta, -alpha, StartTime, time);
            UnmovePiece(Moves[MaxIndex], b);
            
            if(TimeElapsed(StartTime) > time || StopSignal.load()){
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
        cout << "info Depth: " << CurrentDepth << ", Move: " << MoveToStr(OverallBestMove) << endl;
        ++CurrentDepth;
    }
    return OverallBestMove;
}

#endif