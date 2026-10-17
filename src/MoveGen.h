#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Defs.h"
#include "Misc.h"

inline void WKmoves(const chess &b, MoveList &Moves){
    int i = ctz(b.wk);
    int j;
    Bitboard attacks = KingAttacks[i] &~ b.wpcs;
    
    while (attacks) {
        j = ctz(attacks);
        attacks = blsr(attacks);
        Moves.push_back((6 << 15) | j << 6 | i);
    }
    
    // Kingside Castling
    if (b.WCastleKing  &&
        !(b.pieces & ((1ULL << 5) | (1ULL << 6))) &&
        !IsAttacked(b, 4, false) &&
        !IsAttacked(b, 5, false) &&
        !IsAttacked(b, 6, false))
    {
        Moves.push_back((6 << 15) | (6 << 6) | 4);
    }
    // Queenside
    if (b.WCastleQueen &&
        !(b.pieces & ((1ULL << 3) | (1ULL << 2) | (1ULL << 1))) &&
        !IsAttacked(b, 4, false) &&
        !IsAttacked(b, 3, false) &&
        !IsAttacked(b, 2, false))
    {
        Moves.push_back((6 << 15) | (2 << 6) | 4);
    }
}

inline void BKmoves(const chess &b, MoveList &Moves){
    int i = ctz(b.bk);
    int j;
    Bitboard attacks = KingAttacks[i] &~ b.bpcs;
    
    while (attacks) {
        j = ctz(attacks);
        attacks = blsr(attacks);
        Moves.push_back((6 << 15) | j << 6 | i);
    }
    
    // Kingside
    if (b.BCastleKing  &&
        !(b.pieces & ((1ULL << 61) | (1ULL << 62))) &&
        !IsAttacked(b, 60, true) &&
        !IsAttacked(b, 61, true) &&
        !IsAttacked(b, 62, true))
    {
        Moves.push_back((6 << 15) | (62 << 6) | 60);
    }
    // Queenside
    if (b.BCastleQueen &&
        !(b.pieces & ((1ULL << 59) | (1ULL << 58) | (1ULL << 57))) &&
        !IsAttacked(b, 60, true) &&
        !IsAttacked(b, 59, true) &&
        !IsAttacked(b, 58, true))
    {
        Moves.push_back((6 << 15) | (58 << 6) | 60);
    }
}

inline void WNmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.wn;
    Bitboard attacks;
    int i;
    int j;
    while (iter){
        i = ctz(iter);
        iter = blsr(iter);

        attacks = KnightAttacks[i] &~ b.wpcs;
        while (attacks) {
            j = ctz(attacks);
            attacks = blsr(attacks);
            Moves.push_back((2 << 15) | j << 6 | i);
        }
    }
}

inline void BNmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.bn;
    Bitboard attacks;
    int i;
    int j;
    while (iter){
        i = ctz(iter);
        iter = blsr(iter);

        attacks = KnightAttacks[i] &~ b.bpcs;
        while (attacks) {
            j = ctz(attacks);
            attacks = blsr(attacks);
            Moves.push_back((2 << 15) | j << 6 | i);
        }
    }
}

inline void WPmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.wp &~ mask_7;
    int i;
    Bitboard k;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);
        k = 1ULL << i;

        // Single push
        if ((k << 8) &~ b.pieces)
            Moves.push_back((1 << 15) | ctz(k << 8) << 6 | i);
        
        // Double push
        if (((((k & mask_2) << 8) &~ b.pieces) << 8) &~ b.pieces)
            Moves.push_back((1 << 15) | ctz(k << 16) << 6 | i);
        
        // Captures
        if ((k << 7) & b.bpcs & clear_h)
            Moves.push_back((1 << 15) | ctz(k << 7) << 6 | i);
        if ((k << 9) & b.bpcs & clear_a)
            Moves.push_back((1 << 15) | ctz(k << 9) << 6 | i);
        
        // En passant
        if (b.ep >= 0){
            const Bitboard ep = 1ULL << b.ep;
            if ((k << 7) & ep & clear_h) Moves.push_back((7 << 18) | (1 << 15) | b.ep << 6 | i);
            if ((k << 9) & ep & clear_a) Moves.push_back((7 << 18) | (1 << 15) | b.ep << 6 | i);
        }
    }
}

inline void BPmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.bp &~ mask_2;
    int i;
    Bitboard k;

    while(iter){
        i = ctz(iter);
        iter = blsr(iter);
        k = 1ULL << i;

        // Single push
        if ((k >> 8) &~ b.pieces)
            Moves.push_back((1 << 15) | ctz(k >> 8) << 6 | i);
        
        // Double push
        if (((((k & mask_7) >> 8) &~ b.pieces) >> 8) &~ b.pieces)
            Moves.push_back((1 << 15) | ctz(k >> 16) << 6 | i);
        
        // Captures
        if ((k >> 7) & b.wpcs & clear_a)
            Moves.push_back((1 << 15) | ctz(k >> 7) << 6 | i);
        if ((k >> 9) & b.wpcs & clear_h)
            Moves.push_back((1 << 15) | ctz(k >> 9) << 6 | i);
        
        // En passant
        if (b.ep >= 0){
            const Bitboard ep = 1ULL << b.ep;
            if ((k >> 7) & ep & clear_a) Moves.push_back((7 << 18) | (1 << 15) | b.ep << 6 | i);
            if ((k >> 9) & ep & clear_h) Moves.push_back((7 << 18) | (1 << 15) | b.ep << 6 | i);
        }
    }
}

inline void WBmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.wb;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (BishopMovesLookup[i][pext(b.pieces,BishopBlockers[i])] &~ b.wpcs);
        #else
            it = (BishopMovesLookup[i][((b.pieces & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])] &~ b.wpcs);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((3 << 15) | (j << 6) | i);
        }
    }
}

inline void BBmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.bb;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (BishopMovesLookup[i][pext(b.pieces,BishopBlockers[i])] &~ b.bpcs);
        #else
            it = (BishopMovesLookup[i][((b.pieces & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])] &~ b.bpcs);
        #endif
        
        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((3 << 15) | (j << 6) | i);
        }
    }
}

inline void WRmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.wr;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (RookMovesLookup[i][pext(b.pieces,RookBlockers[i])] &~ b.wpcs);
        #else
            it = (RookMovesLookup[i][((b.pieces & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] &~ b.wpcs);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((4 << 15) | (j << 6) | i);
        }
    }
}

inline void BRmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.br;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (RookMovesLookup[i][pext(b.pieces,RookBlockers[i])] &~ b.bpcs);
        #else
            it = (RookMovesLookup[i][((b.pieces & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] &~ b.bpcs);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((4 << 15) | (j << 6) | i);
        }
    }
}

inline void WQmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.wq;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = ((RookMovesLookup[i][pext(b.pieces,RookBlockers[i])] | BishopMovesLookup[i][pext(b.pieces,BishopBlockers[i])]) &~ b.wpcs);
        #else
            it = ((RookMovesLookup[i][((b.pieces & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] | BishopMovesLookup[i][((b.pieces & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])]) &~ b.wpcs);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((5 << 15) | (j << 6) | i);
        }
    }
}

inline void BQmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.bq;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = ((RookMovesLookup[i][pext(b.pieces,RookBlockers[i])] | BishopMovesLookup[i][pext(b.pieces,BishopBlockers[i])]) &~ b.bpcs);
        #else
            it = ((RookMovesLookup[i][((b.pieces & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] | BishopMovesLookup[i][((b.pieces & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])]) &~ b.bpcs);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((5 << 15) | (j << 6) | i);
        }
    }
}

inline MoveList WhiteMoves(const chess &b){
    MoveList moves;

    WKmoves(b,moves);
    WNmoves(b,moves);
    WPmoves(b,moves);
    WBmoves(b,moves);
    WRmoves(b,moves);
    WQmoves(b,moves);

    Bitboard v;
    int i;

    v = b.wp & mask_7;
    while (v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+8))&b.pieces)==0){
            moves.push_back((1 << 15) | (2 << 12) | ((i+8) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i+8) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i+8) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i+8) << 6) | i);
        }
    }

    v = b.wp & mask_7 & clear_a;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+7))&b.bpcs)){
            moves.push_back((1 << 15) | (2 << 12) | ((i+7) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i+7) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i+7) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i+7) << 6) | i);
        }
    }

    v = b.wp & mask_7 & clear_h;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+9))&b.bpcs)){
            moves.push_back((1 << 15) | (2 << 12) | ((i+9) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i+9) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i+9) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i+9) << 6) | i);
        }
    }
    return moves;
}

inline MoveList BlackMoves(const chess &b){
    MoveList moves;

    BKmoves(b,moves);
    BNmoves(b,moves);
    BPmoves(b,moves);
    BBmoves(b,moves);
    BRmoves(b,moves);
    BQmoves(b,moves);

    Bitboard v;
    int i;
    
    v = b.bp & mask_2;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-8))&b.pieces)==0){
            moves.push_back((1 << 15) | (2 << 12) | ((i-8) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i-8) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i-8) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i-8) << 6) | i);
        }
    }

    v = b.bp & mask_2 & clear_h;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-7))&b.wpcs)){
            moves.push_back((1 << 15) | (2 << 12) | ((i-7) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i-7) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i-7) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i-7) << 6) | i);
        }
    }

    v = b.bp&mask_2&clear_a;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-9))&b.wpcs)){
            moves.push_back((1 << 15) | (2 << 12) | ((i-9) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i-9) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i-9) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i-9) << 6) | i);
        }
    }
    return moves;
}

inline MoveList PseudoLegals(const chess &b){
    switch (b.turn){
        case true:
            return WhiteMoves(b);
        case false:
            return BlackMoves(b);
    }
}

#endif