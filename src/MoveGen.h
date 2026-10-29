#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Defs.h"
#include "Misc.h"

inline void WKmoves(const Board &b, MoveList &Moves){
    int i = ctz(b.PieceBitboards[WHITE][KING]);
    int j;
    Bitboard attacks = KingAttacks[i] &~ b.OccupancyColor[WHITE];
    
    while (attacks) {
        j = ctz(attacks);
        attacks = blsr(attacks);
        Moves.push_back((6 << 15) | j << 6 | i);
    }
    
    // Kingside Castling
    if ((b.CastleRights & 8)  &&
        !(b.OccupancyAll & ((1ULL << 5) | (1ULL << 6))) &&
        !IsAttacked(b, 4, BLACK) &&
        !IsAttacked(b, 5, BLACK) &&
        !IsAttacked(b, 6, BLACK))
    {
        Moves.push_back((6 << 15) | (6 << 6) | 4);
    }
    // Queenside
    if ((b.CastleRights & 4) &&
        !(b.OccupancyAll & ((1ULL << 3) | (1ULL << 2) | (1ULL << 1))) &&
        !IsAttacked(b, 4, BLACK) &&
        !IsAttacked(b, 3, BLACK) &&
        !IsAttacked(b, 2, BLACK))
    {
        Moves.push_back((6 << 15) | (2 << 6) | 4);
    }
}

inline void BKmoves(const Board &b, MoveList &Moves){
    int i = ctz(b.PieceBitboards[BLACK][KING]);
    int j;
    Bitboard attacks = KingAttacks[i] &~ b.OccupancyColor[BLACK];
    
    while (attacks) {
        j = ctz(attacks);
        attacks = blsr(attacks);
        Moves.push_back((6 << 15) | j << 6 | i);
    }
    
    // Kingside
    if ((b.CastleRights & 2)  &&
        !(b.OccupancyAll & ((1ULL << 61) | (1ULL << 62))) &&
        !IsAttacked(b, 60, WHITE) &&
        !IsAttacked(b, 61, WHITE) &&
        !IsAttacked(b, 62, WHITE))
    {
        Moves.push_back((6 << 15) | (62 << 6) | 60);
    }
    // Queenside
    if ((b.CastleRights & 1) &&
        !(b.OccupancyAll & ((1ULL << 59) | (1ULL << 58) | (1ULL << 57))) &&
        !IsAttacked(b, 60, WHITE) &&
        !IsAttacked(b, 59, WHITE) &&
        !IsAttacked(b, 58, WHITE))
    {
        Moves.push_back((6 << 15) | (58 << 6) | 60);
    }
}

inline void WNmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[WHITE][KNIGHT];
    Bitboard attacks;
    int i;
    int j;
    while (iter){
        i = ctz(iter);
        iter = blsr(iter);

        attacks = KnightAttacks[i] &~ b.OccupancyColor[WHITE];
        while (attacks) {
            j = ctz(attacks);
            attacks = blsr(attacks);
            Moves.push_back((2 << 15) | j << 6 | i);
        }
    }
}

inline void BNmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[BLACK][KNIGHT];
    Bitboard attacks;
    int i;
    int j;
    while (iter){
        i = ctz(iter);
        iter = blsr(iter);

        attacks = KnightAttacks[i] &~ b.OccupancyColor[BLACK];
        while (attacks) {
            j = ctz(attacks);
            attacks = blsr(attacks);
            Moves.push_back((2 << 15) | j << 6 | i);
        }
    }
}

inline void WPmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[WHITE][PAWN] &~ mask_7;
    int i;
    Bitboard k;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);
        k = 1ULL << i;

        // Single push
        if ((k << 8) &~ b.OccupancyAll)
            Moves.push_back((1 << 15) | ctz(k << 8) << 6 | i);
        
        // Double push
        if (((((k & mask_2) << 8) &~ b.OccupancyAll) << 8) &~ b.OccupancyAll)
            Moves.push_back((1 << 15) | ctz(k << 16) << 6 | i);
        
        // Captures
        if ((k << 7) & b.OccupancyColor[BLACK] & clear_h)
            Moves.push_back((1 << 15) | ctz(k << 7) << 6 | i);
        if ((k << 9) & b.OccupancyColor[BLACK] & clear_a)
            Moves.push_back((1 << 15) | ctz(k << 9) << 6 | i);
        
        // En passant
        if (b.EpSquare >= 0){
            const Bitboard ep = 1ULL << (40 + b.EpSquare);
            if ((k << 7) & ep & clear_h) Moves.push_back((7 << 18) | (1 << 15) | ctz(ep) << 6 | i);
            if ((k << 9) & ep & clear_a) Moves.push_back((7 << 18) | (1 << 15) | ctz(ep) << 6 | i);
        }
    }
}

inline void BPmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[BLACK][PAWN] &~ mask_2;
    int i;
    Bitboard k;

    while(iter){
        i = ctz(iter);
        iter = blsr(iter);
        k = 1ULL << i;

        // Single push
        if ((k >> 8) &~ b.OccupancyAll)
            Moves.push_back((1 << 15) | ctz(k >> 8) << 6 | i);
        
        // Double push
        if (((((k & mask_7) >> 8) &~ b.OccupancyAll) >> 8) &~ b.OccupancyAll)
            Moves.push_back((1 << 15) | ctz(k >> 16) << 6 | i);
        
        // Captures
        if ((k >> 7) & b.OccupancyColor[WHITE] & clear_a)
            Moves.push_back((1 << 15) | ctz(k >> 7) << 6 | i);
        if ((k >> 9) & b.OccupancyColor[WHITE] & clear_h)
            Moves.push_back((1 << 15) | ctz(k >> 9) << 6 | i);
        
        // En passant
        if (b.EpSquare >= 0){
            const Bitboard ep = 1ULL << (16 + b.EpSquare);
            if ((k >> 7) & ep & clear_a) Moves.push_back((7 << 18) | (1 << 15) | ctz(ep) << 6 | i);
            if ((k >> 9) & ep & clear_h) Moves.push_back((7 << 18) | (1 << 15) | ctz(ep) << 6 | i);
        }
    }
}

inline void WBmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[WHITE][BISHOP];
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (BishopMovesLookup[i][pext(b.OccupancyAll,BishopBlockers[i])] &~ b.OccupancyColor[WHITE]);
        #else
            it = (BishopMovesLookup[i][((b.OccupancyAll & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])] &~ b.OccupancyColor[WHITE]);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((3 << 15) | (j << 6) | i);
        }
    }
}

inline void BBmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[BLACK][BISHOP];
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (BishopMovesLookup[i][pext(b.OccupancyAll,BishopBlockers[i])] &~ b.OccupancyColor[BLACK]);
        #else
            it = (BishopMovesLookup[i][((b.OccupancyAll & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])] &~ b.OccupancyColor[BLACK]);
        #endif
        
        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((3 << 15) | (j << 6) | i);
        }
    }
}

inline void WRmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[WHITE][ROOK];
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (RookMovesLookup[i][pext(b.OccupancyAll,RookBlockers[i])] &~ b.OccupancyColor[WHITE]);
        #else
            it = (RookMovesLookup[i][((b.OccupancyAll & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] &~ b.OccupancyColor[WHITE]);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((4 << 15) | (j << 6) | i);
        }
    }
}

inline void BRmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[BLACK][ROOK];
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = (RookMovesLookup[i][pext(b.OccupancyAll,RookBlockers[i])] &~ b.OccupancyColor[BLACK]);
        #else
            it = (RookMovesLookup[i][((b.OccupancyAll & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] &~ b.OccupancyColor[BLACK]);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((4 << 15) | (j << 6) | i);
        }
    }
}

inline void WQmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[WHITE][QUEEN];
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = ((RookMovesLookup[i][pext(b.OccupancyAll,RookBlockers[i])] | BishopMovesLookup[i][pext(b.OccupancyAll,BishopBlockers[i])]) &~ b.OccupancyColor[WHITE]);
        #else
            it = ((RookMovesLookup[i][((b.OccupancyAll & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] | BishopMovesLookup[i][((b.OccupancyAll & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])]) &~ b.OccupancyColor[WHITE]);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((5 << 15) | (j << 6) | i);
        }
    }
}

inline void BQmoves(const Board &b, MoveList &Moves){
    Bitboard iter = b.PieceBitboards[BLACK][QUEEN];
    int i;
    while(iter){
        i = ctz(iter);
        iter = blsr(iter);

        Bitboard it;
        #ifdef USE_BMI2
            it = ((RookMovesLookup[i][pext(b.OccupancyAll,RookBlockers[i])] | BishopMovesLookup[i][pext(b.OccupancyAll,BishopBlockers[i])]) &~ b.OccupancyColor[BLACK]);
        #else
            it = ((RookMovesLookup[i][((b.OccupancyAll & RookBlockers[i]) * RookMagics[i] >> RookShifts[i])] | BishopMovesLookup[i][((b.OccupancyAll & BishopBlockers[i]) * BishopMagics[i] >> BishopShifts[i])]) &~ b.OccupancyColor[BLACK]);
        #endif

        int j;
        while (it){
            j = ctz(it);
            it = blsr(it);

            Moves.push_back((5 << 15) | (j << 6) | i);
        }
    }
}

inline MoveList WhiteMoves(const Board &b){
    MoveList moves;

    WKmoves(b,moves);
    WNmoves(b,moves);
    WPmoves(b,moves);
    WBmoves(b,moves);
    WRmoves(b,moves);
    WQmoves(b,moves);

    Bitboard v;
    int i;

    v = b.PieceBitboards[WHITE][PAWN] & mask_7;
    while (v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+8))&b.OccupancyAll)==0){
            moves.push_back((1 << 15) | (2 << 12) | ((i+8) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i+8) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i+8) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i+8) << 6) | i);
        }
    }

    v = b.PieceBitboards[WHITE][PAWN] & mask_7 & clear_a;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+7))&b.OccupancyColor[BLACK])){
            moves.push_back((1 << 15) | (2 << 12) | ((i+7) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i+7) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i+7) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i+7) << 6) | i);
        }
    }

    v = b.PieceBitboards[WHITE][PAWN] & mask_7 & clear_h;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i+9))&b.OccupancyColor[BLACK])){
            moves.push_back((1 << 15) | (2 << 12) | ((i+9) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i+9) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i+9) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i+9) << 6) | i);
        }
    }
    return moves;
}

inline MoveList BlackMoves(const Board &b){
    MoveList moves;

    BKmoves(b,moves);
    BNmoves(b,moves);
    BPmoves(b,moves);
    BBmoves(b,moves);
    BRmoves(b,moves);
    BQmoves(b,moves);

    Bitboard v;
    int i;
    
    v = b.PieceBitboards[BLACK][PAWN] & mask_2;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-8))&b.OccupancyAll)==0){
            moves.push_back((1 << 15) | (2 << 12) | ((i-8) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i-8) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i-8) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i-8) << 6) | i);
        }
    }

    v = b.PieceBitboards[BLACK][PAWN] & mask_2 & clear_h;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-7))&b.OccupancyColor[WHITE])){
            moves.push_back((1 << 15) | (2 << 12) | ((i-7) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i-7) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i-7) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i-7) << 6) | i);
        }
    }

    v = b.PieceBitboards[BLACK][PAWN] & mask_2 & clear_a;
    while(v){
        i = ctz(v);
        v = blsr(v);
        if(((1ULL<<(i-9))&b.OccupancyColor[WHITE])){
            moves.push_back((1 << 15) | (2 << 12) | ((i-9) << 6) | i);
            moves.push_back((1 << 15) | (3 << 12) | ((i-9) << 6) | i);
            moves.push_back((1 << 15) | (4 << 12) | ((i-9) << 6) | i);
            moves.push_back((1 << 15) | (5 << 12) | ((i-9) << 6) | i);
        }
    }
    return moves;
}

inline MoveList PseudoLegals(const Board &b){
    if (b.turn == WHITE) return WhiteMoves(b);
    return BlackMoves(b);
}

#endif