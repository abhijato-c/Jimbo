#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Defs.h"
#include "MoveGen.h"
#include "MovePiece.h"
#include "Misc.h"

inline int ScoreMove(const Board &b, Move m, Move pv = 0){
    if (pv && (m & MOVE_MASK) == (pv & MOVE_MASK)) return INT_MAX - 1;

    int victim   = (((m >> 18) & 7) == EP) ? PAWN : b.OccupancyType[(m >> 6) & 63];
    int attacker = (m >> 15) & 7;
    int promo    = (m >> 12) & 7;

    int score = 0;
    if (victim) score += 10000 + 10 * victim - attacker;
    if (promo)  score += 9000 + promo;
    return score;
}

inline void PickNext(MoveList &moves, int *scores, int n, int i){
    int best = i;
    for (int j = i + 1; j < n; ++j)
        if (scores[j] > scores[best]) best = j;
    swap(moves[i],  moves[best]);
    swap(scores[i], scores[best]);
}

inline int MiniMax(Board &b, int depth, int alpha, int beta, timept start, int64_t time){
    ++NodeCount;
    if (depth == 1) return StaticEval(b);

    MoveList Moves = PseudoLegals(b);
    bool LegalFound = false;
    int n = Moves.size();
    int scores[256];
    int nPos = 0;
    for (int i = 0; i < n; ++i) {
        scores[i] = ScoreMove(b, Moves[i]); 
        nPos += (scores[i] > 0); 
    }

    for (int i = 0; i < n; ++i){
        if (nPos > 0) {
            PickNext(Moves, scores, n, i); 
            --nPos; 
        }
        Move &m = Moves[i];
        if (!MovePiece(b, m)) continue;
        LegalFound = true;
        alpha = max(-MiniMax(b, depth - 1, -beta, -alpha, start, time), alpha);
        UnmovePiece(b, m);

        if (alpha >= beta) return alpha;
        if ((NodeCount & 2047) == 0 && (TimeElapsed(start) > time || StopSignal.load())) return alpha;
    }

    if (!LegalFound){
        if (IsAttacked(b, ctz(b.PieceBitboards[b.turn][KING]), b.turn ^ 1))
            return (-MATE_BOUND - depth);
        return 0;
    }
    return alpha;
}

inline Move IterativeDeepening(Board &b, int64_t time, int Mdepth){
    timept StartTime = chrono::steady_clock::now();
    Move OverallBestMove = 0;
    int CurrentDepth = 1;
    int BestEval = -INT_MAX;

    while (TimeElapsed(StartTime) < time && CurrentDepth <= Mdepth && !StopSignal.load() && !(abs(BestEval) >= MATE_BOUND && CurrentDepth > 2)){
        NodeCount = 0;
        Move DepthBestMove = 0;
        int alpha = -INT_MAX;
        bool LegalFound = false;
        bool Aborted = false;

        MoveList Moves = PseudoLegals(b);
        int n = Moves.size();
        int scores[256];
        int nPos = 0;
        
        for (int i = 0; i < n; ++i) {
            scores[i] = ScoreMove(b, Moves[i], OverallBestMove); 
            nPos += (scores[i] > 0); 
        }

        for (int i = 0; i < n; ++i){
            if (nPos > 0) {
                PickNext(Moves, scores, n, i); 
                --nPos; 
            }
            Move &m = Moves[i];
            if (!MovePiece(b, m)) continue;
            LegalFound = true;
            int eval = -MiniMax(b, CurrentDepth, -INT_MAX, -alpha, StartTime, time);
            UnmovePiece(b, m);

            if (TimeElapsed(StartTime) > time || StopSignal.load()) {
                Aborted = true; 
                break; 
            }

            if (eval > alpha){
                alpha = eval;
                DepthBestMove = m;
            }
        }

        if (!LegalFound) break;

        if (Aborted){
            if (DepthBestMove) OverallBestMove = DepthBestMove;
            break;
        }

        BestEval = alpha;
        OverallBestMove = DepthBestMove;
        cout << "info depth " << CurrentDepth << " score cp " << (BestEval / 24) << " nodes " << NodeCount << " pv " << MoveToStr(OverallBestMove) << endl;
        ++CurrentDepth;
    }

    if (OverallBestMove == 0){
        MoveList Moves = PseudoLegals(b);
        for (int i = 0; i < Moves.size(); ++i){
            Move m = Moves[i];
            if (MovePiece(b, m)) {
                UnmovePiece(b, m); 
                return m; 
            }
        }
    }
    return OverallBestMove;
}

#endif