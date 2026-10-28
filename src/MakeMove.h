#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Defs.h"
#include "MoveGen.h"
#include "MovePiece.h"
#include "Misc.h"

// TODO - EARLY CAPTURE INCENTIVE (PROMOTION)

inline int MiniMax(chess &b, int depth, int alpha, int beta, timept start, int64_t time){
    if(depth == 1) return StaticEval(b);

    MoveList Moves = PseudoLegals(b);
    bool LegalFound = false;

    // Compute static evals of each position for move ordering
    int StaticEvals[Moves.size()];
    for (int i = 0; i < Moves.size(); ++i){
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
    for(int i = 0; i < Moves.size(); ++i){
        // Find position with highest static eval
        int MaxEval = StaticEvals[0];
        int MaxIndex = 0;
        for(int j = 1; j < Moves.size(); ++j){
            if(StaticEvals[j] > MaxEval){ 
                MaxEval = StaticEvals[j]; 
                MaxIndex = j;
            }
        }
        StaticEvals[MaxIndex] = -INT_MAX;
        if (!MovePiece(Moves[MaxIndex], b)) continue;
        LegalFound = true;
        alpha = max(-MiniMax(b, depth-1, -beta, -alpha, start, time), alpha);
        UnmovePiece(Moves[MaxIndex], b);

        if (alpha >= beta) return alpha;
        if ((++NodeCount & 2047) == 0 && (TimeElapsed(start) > time || StopSignal.load())) return alpha;
    }

    if (!LegalFound){
        // Checkmate
        if (IsAttacked(b, ctz(b.turn ? b.wk : b.bk), b.turn ? false : true)) return (-MATE_BOUND - depth);
        // Stalemate
        else return 0;
    }
    return alpha;
}

inline Move IterativeDeepening(chess &b, int64_t time, int Mdepth){
    timept StartTime = chrono::steady_clock::now();
    Move OverallBestMove = 0;
    int CurrentDepth = 1;
    int BestEval = -INT_MAX;
    while (TimeElapsed(StartTime) < time && CurrentDepth <= Mdepth && !StopSignal.load() && !(abs(BestEval) >= MATE_BOUND && CurrentDepth > 2)){
        NodeCount = 0;
        int eval;
        int DepthBestMove = 0;
        int alpha = -INT_MAX;
        int beta = INT_MAX;
        bool LegalFound = false;
        MoveList Moves = PseudoLegals(b);

        // Compute static evals of each position for move ordering
        int StaticEvals[Moves.size()];
        for (int i = 0; i < Moves.size(); ++i){
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
        for(int i = 0; i < Moves.size(); ++i){
            // Find position with highest static eval
            int MaxEval = StaticEvals[0];
            int MaxIndex = 0;
            for(int j = 1; j < Moves.size(); ++j){
                if(StaticEvals[j] > MaxEval){ 
                    MaxEval = StaticEvals[j]; 
                    MaxIndex = j;
                }
            }
            StaticEvals[MaxIndex] = -INT_MAX;
            if (!MovePiece(Moves[MaxIndex], b)) continue;
            LegalFound = true;
            eval = -MiniMax(b, CurrentDepth, -beta, -alpha, StartTime, time);
            UnmovePiece(Moves[MaxIndex], b);
            
            if (TimeElapsed(StartTime) > time || StopSignal.load()){
                if(alpha > BestEval) return DepthBestMove; 
                return OverallBestMove;
            }
            if (eval > alpha){
                alpha = eval;
                DepthBestMove = Moves[MaxIndex];
            }
            if (alpha >= beta) break;
        }

        if (!LegalFound) break;

        BestEval = alpha;
        OverallBestMove = DepthBestMove;
        cout << "info depth " << CurrentDepth << " move " << MoveToStr(OverallBestMove) << endl;
        ++CurrentDepth;
    }
    return OverallBestMove;
}

#endif