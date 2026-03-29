#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Defs.h"

inline bool IsAttacked(const chess &b, int sq, bool byWhite){
    const Bitboard sqBB = 1ULL << sq;

    const Bitboard ka =
        ((sqBB & clear_a & clear_b) >> 6) | ((sqBB & clear_a) >> 15) |
        ((sqBB & clear_h) >> 17)          | ((sqBB & clear_h & clear_g) >> 10) |
        ((sqBB & clear_h & clear_g) << 6) | ((sqBB & clear_h) << 15) |
        ((sqBB & clear_a) << 17)          | ((sqBB & clear_a & clear_b) << 10);

    const Bitboard king_att =
        ((sqBB & clear_h) << 7) | (sqBB << 8) | ((sqBB & clear_a) << 9) |
        ((sqBB & clear_a) << 1) | ((sqBB & clear_a) >> 7) | (sqBB >> 8) |
        ((sqBB & clear_h) >> 9) | ((sqBB & clear_h) >> 1);

    #ifdef USE_BMI2
        const Bitboard diag = BishopMovesLookup[sq][pext(b.pieces, BishopBlockers[sq])];
        const Bitboard orth = RookMovesLookup[sq][pext(b.pieces, RookBlockers[sq])];
    #else
        const Bitboard diag = BishopMovesLookup[sq][
            ((b.pieces & BishopBlockers[sq]) * BishopMagics[sq]) >> BishopShifts[sq]];
        const Bitboard orth = RookMovesLookup[sq][
            ((b.pieces & RookBlockers[sq])   * RookMagics[sq])   >> RookShifts[sq]];
    #endif
 
    if (byWhite){
        if (b.wp & (((sqBB & clear_a) >> 7) | ((sqBB & clear_h) >> 9))) return true;
        if (b.wn & ka)               return true;
        if ((b.wb | b.wq) & diag)    return true;
        if ((b.wr | b.wq) & orth)    return true;
        if (b.wk & king_att)         return true;
    } 
    else {
        if (b.bp & (((sqBB & clear_h) << 7) | ((sqBB & clear_a) << 9))) return true;
        if (b.bn & ka)               return true;
        if ((b.bb | b.bq) & diag)    return true;
        if ((b.br | b.bq) & orth)    return true;
        if (b.bk & king_att)         return true;
    }
    return false;
}

inline void WKmoves(const chess &b, MoveList &Moves){
    int i = ctz(b.wk);
    
    if ((((1ULL << i) & clear_h) << 7) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 7) << 6 | i);
    if (((1ULL << i) << 8) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 8) << 6 | i);
    if ((((1ULL << i) & clear_a) << 9) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 9) << 6 | i);
    if ((((1ULL << i) & clear_a) << 1) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 1) << 6 | i);
    if ((((1ULL << i) & clear_a) >> 7) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 7) << 6 | i);
    if (((1ULL << i) >> 8) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 8) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 9) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 9) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 1) &~ b.wpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 1) << 6 | i);
    
    // Kingside Castling
    if (b.WCastleKing  &&
        !(b.pieces & ((1ULL<<2)|(1ULL<<1))) &&
        !IsAttacked(b, 3, false) &&
        !IsAttacked(b, 2, false) &&
        !IsAttacked(b, 1, false))
    {
        Moves.push_back((5<<15) | (1<<6) | 3);
    }
    // Queenside
    if (b.WCastleQueen &&
        !(b.pieces & ((1ULL<<4)|(1ULL<<5)|(1ULL<<6))) &&
        !IsAttacked(b, 3, false) &&
        !IsAttacked(b, 4, false) &&
        !IsAttacked(b, 5, false))
    {
        Moves.push_back((5<<15) | (5<<6) | 3);
    }
}

inline void BKmoves(const chess &b, MoveList &Moves){
    int i = ctz(b.bk);

    if ((((1ULL << i) & clear_h) << 7) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 7) << 6 | i);
    if (((1ULL << i) << 8) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 8) << 6 | i);
    if ((((1ULL << i) & clear_a) << 9) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 9) << 6 | i);
    if ((((1ULL << i) & clear_a) << 1) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) << 1) << 6 | i);
    if ((((1ULL << i) & clear_a) >> 7) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 7) << 6 | i);
    if (((1ULL << i) >> 8) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 8) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 9) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 9) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 1) &~ b.bpcs)
        Moves.push_back((5 << 15) | ctz((1ULL << i) >> 1) << 6 | i);
    
    // Kingside
    if (b.BCastleKing  &&
        !(b.pieces & ((1ULL<<58)|(1ULL<<57))) &&
        !IsAttacked(b, 59, true) &&
        !IsAttacked(b, 58, true) &&
        !IsAttacked(b, 57, true))
    {
        Moves.push_back((5<<15)|(57<<6)|59);
    }
    // Queenside
    if (b.BCastleQueen &&
        !(b.pieces & ((1ULL<<60)|(1ULL<<61)|(1ULL<<62))) &&
        !IsAttacked(b, 59, true) &&
        !IsAttacked(b, 60, true) &&
        !IsAttacked(b, 61, true))
    {
        Moves.push_back((5<<15)|(61<<6)|59);
    }
}

inline void WNmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.wn;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        if ((((1ULL << i) & clear_a & clear_b) >> 6) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 6) << 6 | i);
        if ((((1ULL << i) & clear_a) >> 15) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 15) << 6 | i);
        if ((((1ULL << i) & clear_h) >> 17) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 17) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) >> 10) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 10) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) << 6) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 6) << 6 | i);
        if ((((1ULL << i) & clear_h) << 15) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 15) << 6 | i);
        if ((((1ULL << i) & clear_a) << 17) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 17) << 6 | i);
        if ((((1ULL << i) & clear_a & clear_b) << 10) &~ b.wpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 10) << 6 | i);
    }
}

inline void BNmoves(const chess &b, MoveList &Moves){
    Bitboard iter = b.bn;
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        if ((((1ULL << i) & clear_a & clear_b) >> 6) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 6) << 6 | i);
        if ((((1ULL << i) & clear_a) >> 15) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 15) << 6 | i);
        if ((((1ULL << i) & clear_h) >> 17) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 17) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) >> 10) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) >> 10) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) << 6) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 6) << 6 | i);
        if ((((1ULL << i) & clear_h) << 15) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 15) << 6 | i);
        if ((((1ULL << i) & clear_a) << 17) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 17) << 6 | i);
        if ((((1ULL << i) & clear_a & clear_b) << 10) &~ b.bpcs)
            Moves.push_back((1 << 15) | ctz((1ULL << i) << 10) << 6 | i);
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
            Moves.push_back(ctz(k << 8) << 6 | i);
        
        // Double push
        if ((((((1ULL << i) & mask_2) << 8) &~ b.pieces) << 8) &~ b.pieces)
            Moves.push_back(ctz(k << 16) << 6 | i);
        
        // Captures
        if ((k << 7) & b.bpcs & clear_a)
            Moves.push_back(ctz(k << 7) << 6 | i);
        if ((k << 9) & b.bpcs & clear_h)
            Moves.push_back(ctz(k << 9) << 6 | i);
        
        // En passant
        if (b.ep >= 0){
            const Bitboard ep = 1ULL << b.ep;
            if ((k << 7) & ep & clear_a) Moves.push_back(b.ep << 6 | i);
            if ((k << 9) & ep & clear_h) Moves.push_back(b.ep << 6 | i);
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
            Moves.push_back(ctz(k >> 8) << 6 | i);
        
        // Double push
        if (((((k & mask_7) >> 8) &~ b.pieces) >> 8) &~ b.pieces)
            Moves.push_back(ctz(k >> 16) << 6 | i);
        
        // Captures
        if ((k >> 7) & b.wpcs & clear_h)
            Moves.push_back(ctz(k >> 7) << 6 | i);
        if ((k >> 9) & b.wpcs & clear_a)
            Moves.push_back(ctz(k >> 9) << 6 | i);
        
        // En passant
        if (b.ep >= 0){
            const Bitboard ep = 1ULL << b.ep;
            if ((k >> 7) & ep & clear_h) Moves.push_back(b.ep << 6 | i);
            if ((k >> 9) & ep & clear_a) Moves.push_back(b.ep << 6 | i);
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
        while(it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((2 << 15) | (j << 6) | i);
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
        while(it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((2 << 15) | (j << 6) | i);
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
        while(it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((3 << 15) | (j << 6) | i);
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
        while(it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((3 << 15) | (j << 6) | i);
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
        while(it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((4 << 15) | (j << 6) | i);
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
        while(it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((4 << 15) | (j << 6) | i);
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
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+8))&b.pieces)==0){
            moves.push_back((1 << 12) | ((i+8) << 6) | i);
            moves.push_back((2 << 12) | ((i+8) << 6) | i);
            moves.push_back((3 << 12) | ((i+8) << 6) | i);
            moves.push_back((4 << 12) | ((i+8) << 6) | i);
        }
    }

    v = b.wp & mask_7 & clear_h;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+7))&b.bpcs)){
            moves.push_back((1 << 12) | ((i+7) << 6) | i);
            moves.push_back((2 << 12) | ((i+7) << 6) | i);
            moves.push_back((3 << 12) | ((i+7) << 6) | i);
            moves.push_back((4 << 12) | ((i+7) << 6) | i);
        }
    }

    v = b.wp & mask_7 & clear_a;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+9))&b.bpcs)){
            moves.push_back((1 << 12) | ((i+9) << 6) | i);
            moves.push_back((2 << 12) | ((i+9) << 6) | i);
            moves.push_back((3 << 12) | ((i+9) << 6) | i);
            moves.push_back((4 << 12) | ((i+9) << 6) | i);
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
            moves.push_back((1 << 12) | ((i-8) << 6) | i);
            moves.push_back((2 << 12) | ((i-8) << 6) | i);
            moves.push_back((3 << 12) | ((i-8) << 6) | i);
            moves.push_back((4 << 12) | ((i-8) << 6) | i);
        }
    }

    v = b.bp & mask_2 & clear_a;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-7))&b.wpcs)){
            moves.push_back((1 << 12) | ((i-7) << 6) | i);
            moves.push_back((2 << 12) | ((i-7) << 6) | i);
            moves.push_back((3 << 12) | ((i-7) << 6) | i);
            moves.push_back((4 << 12) | ((i-7) << 6) | i);
        }
    }

    v = b.bp&mask_2&clear_h;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-9))&b.wpcs)){
            moves.push_back((1 << 12) | ((i-9) << 6) | i);
            moves.push_back((2 << 12) | ((i-9) << 6) | i);
            moves.push_back((3 << 12) | ((i-9) << 6) | i);
            moves.push_back((4 << 12) | ((i-9) << 6) | i);
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