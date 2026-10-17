#ifndef MOVEPIECE_H
#define MOVEPIECE_H

#include "Defs.h"

inline void MovePieceWhite(Move& mv, chess& b){
    Bitboard to = (1ULL << ((mv>>6) & 63));
    Bitboard f = (1ULL << (mv & 63));

    if ((b.pieces & to) != 0){
        if((b.bp & to) != 0){
            b.bp &= ~to;
            mv |= (1 << 18);
        }
        else if((b.bn & to) != 0){
            b.bn &= ~to;
            mv |= (2 << 18);
        }
        else if ((b.bb & to) != 0){
            b.bb &= ~to;
            mv |= (3 << 18);
        }
        else if((b.br & to) != 0){
            b.br &= ~to;
            mv |= (4 << 18);
        }
        else if((b.bq & to) != 0){
            b.bq &= ~to;
            mv |= (5 << 18);
        }
        else if((b.bk & to) != 0){
            b.bk &= ~to;
            mv |= (6 << 18);
        }
    }

    // Move piece
    switch ((mv >> 15) & 7){
        case 0:
            b.wp = (b.wp & ~f) | to;
            break;
        case 1:
            b.wn = (b.wn & ~f) | to;;
            break;
        case 2:
            b.wb = (b.wb & ~f) | to;
            break;
        case 3:
            b.wr = (b.wr & ~f) | to;
            if ((mv & 63) == 0) b.WCastleKing = false;
            else if ((mv & 63) == 7) b.WCastleQueen = false;
            break;
        case 4:
            b.wq = (b.wq & ~f) | to;
            break;
        case 5:
            b.wk = (b.wk & ~f) | to;
            b.WCastleKing = false;
            b.WCastleQueen = false;
            break;
    }

    // Promotion
    switch(((mv >> 12) & 7)){
        case 1:
            b.wp &= ~to;
            b.wn |= to;
            break;
        case 2:
            b.wp &= ~to;
            b.wb |= to;
            break;
        case 3:
            b.wp &= ~to;
            b.wr |= to;
            break;
        case 4:
            b.wp &= ~to;
            b.wq |= to;
            break;
    }
    
    b.turn=false;
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline void MovePieceBlack(Move& mv, chess& b){
    Bitboard to = (1ULL << ((mv>>6) & 63));
    Bitboard f = (1ULL << (mv & 63));

    if ((b.pieces & to) != 0){
        if((b.wp & to) != 0){
            b.wp &= ~to;
            mv |= (1 << 18);
        }
        else if((b.wn & to) != 0){
            b.wn &= ~to;
            mv |= (2 << 18);
        }
        else if ((b.wb & to) != 0){
            b.wb &= ~to;
            mv |= (3 << 18);
        }
        else if((b.wr & to) != 0){
            b.wr &= ~to;
            mv |= (4 << 18);
        }
        else if((b.wq & to) != 0){
            b.wq &= ~to;
            mv |= (5 << 18);
        }
        else if((b.wk & to) != 0){
            b.wk &= ~to;
            mv |= (6 << 18);
        }
    }

    // Move piece
    switch ((mv >> 15) & 7){
        case 0:
            b.bp = (b.bp & ~f) | to;
            break;
        case 1:
            b.bn = (b.bn & ~f) | to;
            break;
        case 2:
            b.bb = (b.bb & ~f) | to;
            break;
        case 3:
            b.br = (b.br & ~f) | to;
            if ((mv & 63) == 56) b.BCastleKing = false;
            else if ((mv & 63) == 63) b.BCastleQueen = false;
            break;
        case 4:
            b.bq = (b.bq & ~f) | to;
            break;
        case 5:
            b.bk = (b.bk & ~f) | to;
            b.BCastleKing = false;
            b.BCastleQueen = false;
            break;
    }

    // Promotion
    switch(((mv >> 12) & 7)){
        case 1:
            b.bp &= ~to;
            b.bn |= to;
            break;
        case 2:
            b.bp &= ~to;
            b.bb |= to;
            break;
        case 3:
            b.bp &= ~to;
            b.br |= to;
            break;
        case 4:
            b.bp &= ~to;
            b.bq |= to;
            break;
    }

    b.turn=true;
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline void MovePiece(Move& mv, chess& b){
    switch(b.turn){
        case true:
            return MovePieceWhite(mv,b);
        case false:
            return MovePieceBlack(mv,b);
    }
}

inline void UnMoveWhite(const Move mv, chess &b){
    Bitboard f = (1ULL << (mv & 63));
    Bitboard to = (1ULL << ((mv>>6) & 63));

    // Move back piece
    switch ((mv >> 15) & 7){
        case 0:
            b.wp = (b.wp & ~to) | f;
            break;
        case 1:
            b.wn = (b.wn & ~to) | f;
            break;
        case 2:
            b.wb = (b.wb & ~to) | f;
            break;
        case 3:
            b.wr = (b.wr & ~to) | f;
            break;
        case 4:
            b.wq = (b.wq & ~to) | f;
            break;
        case 5:
            b.wk = (b.wk & ~to) | f;
            break;
    }
     
    // Undo promotion
    switch (((mv >> 12) & 7)){
        case 1:
            b.wn &= ~to;
            break;
        case 2:
            b.wb &= ~to;
            break;
        case 3:
            b.wr &= ~to;
            break;
        case 4:
            b.wq &= ~to;
            break;
    }

    // Undo capture
    switch (((mv >> 18) & 7)){
        case 1:
            b.bp |= to;
            break;
        case 2:
            b.bn |= to;
            break;
        case 3:
            b.bb |= to;
            break;
        case 4:
            b.br |= to;
            break;
        case 5:
            b.bq |= to;
            break;
        case 6:
            b.bk |= to;
            break;
    }
}

inline void UnMoveBlack(const Move mv, chess &b){
    Bitboard f = (1ULL << (mv & 63));
    Bitboard to = (1ULL << ((mv>>6) & 63));

    // Move back piece
    switch ((mv >> 15) & 7){
        case 0:
            b.bp = (b.bp & ~to) | f;
            break;
        case 1:
            b.bn = (b.bn & ~to) | f;
            break;
        case 2:
            b.bb = (b.bb & ~to) | f;
            break;
        case 3:
            b.br = (b.br & ~to) | f;
            break;
        case 4:
            b.bq = (b.bq & ~to) | f;
            break;
        case 5:
            b.bk = (b.bk & ~to) | f;
            break;
    }
     
    // Undo promotion
    switch (((mv >> 12) & 7)){
        case 1:
            b.bn &= ~to;
            break;
        case 2:
            b.bb &= ~to;
            break;
        case 3:
            b.br &= ~to;
            break;
        case 4:
            b.bq &= ~to;
            break;
    }

    // Undo capture
    switch (((mv >> 18) & 7)){
        case 1:
            b.wp |= to;
            break;
        case 2:
            b.wn |= to;
            break;
        case 3:
            b.wb |= to;
            break;
        case 4:
            b.wr |= to;
            break;
        case 5:
            b.wq |= to;
            break;
        case 6:
            b.wk |= to;
            break;
    }
}

inline void UnmovePiece(const Move mv, chess &b){
    if (b.turn) UnMoveBlack(mv, b);
    else UnMoveWhite(mv, b);

    b.turn = !b.turn;
    b.bpcs = b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs = b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces = b.bpcs|b.wpcs;
}

#endif