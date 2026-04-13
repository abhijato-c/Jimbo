#ifndef MOVEPIECE_H
#define MOVEPIECE_H

#include "Defs.h"

inline void MoveWhitePiece(Move& mv, chess& b){
    Bitboard to = (1ULL << ((mv>>6) & 63));
    Bitboard f = (1ULL << (mv & 63));

    // Capture
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

        // Remove castle rights
        if (to == (1ULL << 0))
            b.WCastleKing  = false;
        else if (to == (1ULL << 7)) 
            b.WCastleQueen = false;
        else if (to == (1ULL << 3)){
            b.WCastleKing = false;
            b.WCastleQueen = false;
        }
    }

    // Move piece
    switch ((mv >> 15) & 7){
        case 0:
            b.wp = (b.wp & ~f) | to;

            // En passant
            if (((mv >> 18) & 7) == 7)
                b.bp &= ~(to >> 8);
            if ((ctz(to) - ctz(f)) == 16)
                b.ep = ctz(to) - 8;
            break;
        case 1:
            b.wn = (b.wn & ~f) | to;
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

            // Castle
            if (ctz(f) == 3){
                // Kingside
                if (ctz(to) == 1 || ctz(to) == 0) b.wr = (b.wr & ~(1ULL << 0)) | (1ULL << 2);
                //Queen
                else if (ctz(to) == 5 || ctz(to) == 7) b.wr = (b.wr & ~(1ULL << 7)) | (1ULL << 4);
            }
            b.WCastleKing = false;
            b.WCastleQueen = false;
            break;
    }

    // Promotion
    switch(((mv >> 12) & 7)){
        case 1: b.wp &= ~to; b.wn |= to; break;
        case 2: b.wp &= ~to; b.wb |= to; break;
        case 3: b.wp &= ~to; b.wr |= to; break;
        case 4: b.wp &= ~to; b.wq |= to; break;
    }
}

inline void MoveBlackPiece(Move& mv, chess& b){
    Bitboard to = (1ULL << ((mv>>6) & 63));
    Bitboard f = (1ULL << (mv & 63));

    // Capture
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

        // Remove castle rights
        if (to == (1ULL << 56))
            b.BCastleKing  = false;
        else if (to == (1ULL << 63)) 
            b.BCastleQueen = false;
        else if (to == (1ULL << 59)){
            b.BCastleKing = false;
            b.BCastleQueen = false;
        }
    }

    // Move piece
    switch ((mv >> 15) & 7){
        case 0:
            b.bp = (b.bp & ~f) | to;

            // En passant
            if (((mv >> 18) & 7) == 7)
                b.wp &= ~(to << 8);
            if ((ctz(f) - ctz(to)) == 16)
                b.ep = ctz(to) + 8;
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

            // Castle
            if (ctz(f) == 59){
                // Kingside
                if (ctz(to) == 57 || ctz(to) == 56) b.br = (b.br & ~(1ULL << 56)) | (1ULL << 58);
                //Queen
                else if (ctz(to) == 61 || ctz(to) == 63) b.br = (b.br & ~(1ULL << 63)) | (1ULL << 60);
            }
            b.BCastleKing = false;
            b.BCastleQueen = false;
            break;
    }

    // Promotion
    switch(((mv >> 12) & 7)){
        case 1: b.bp &= ~to; b.bn |= to; break;
        case 2: b.bp &= ~to; b.bb |= to; break;
        case 3: b.bp &= ~to; b.br |= to; break;
        case 4: b.bp &= ~to; b.bq |= to; break;
    }
}

inline void MovePiece(Move& mv, chess& b){
    mv |= (uint32_t)(b.ep + 1) << 25;
    mv |= ((uint32_t)(b.WCastleKing << 3) | (b.WCastleQueen << 2) | (b.BCastleKing << 1) | b.BCastleQueen) << 21;
    b.ep = -1;
    
    if(b.turn) MoveWhitePiece(mv,b);
    else       MoveBlackPiece(mv,b);
    
    b.turn = !b.turn;
    b.bpcs = b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs = b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces = b.bpcs|b.wpcs;
}

inline void UnMoveWhite(const Move mv, chess &b){
    Bitboard f = (1ULL << (mv & 63));
    Bitboard to = (1ULL << ((mv>>6) & 63));

    // Move back piece
    switch ((mv >> 15) & 7){
        case 0: b.wp = (b.wp & ~to) | f; break;
        case 1: b.wn = (b.wn & ~to) | f; break;
        case 2: b.wb = (b.wb & ~to) | f; break;
        case 3: b.wr = (b.wr & ~to) | f; break;
        case 4: b.wq = (b.wq & ~to) | f; break;
        case 5:
            b.wk = (b.wk & ~to) | f;

            // Castle
            if (ctz(f) == 3){
                // Kingside
                if (ctz(to) == 1 || ctz(to) == 0) b.wr = (b.wr & ~(1ULL << 2)) | (1ULL << 0);
                //Queen
                else if (ctz(to) == 5 || ctz(to) == 7) b.wr = (b.wr & ~(1ULL << 4)) | (1ULL << 7);
            }
            break;
    }
     
    // Undo promotion
    switch (((mv >> 12) & 7)){
        case 1: b.wn &= ~to; break;
        case 2: b.wb &= ~to; break;
        case 3: b.wr &= ~to; break;
        case 4: b.wq &= ~to; break;
    }

    // Undo capture
    switch (((mv >> 18) & 7)){
        case 1: b.bp |= to; break;
        case 2: b.bn |= to; break;
        case 3: b.bb |= to; break;
        case 4: b.br |= to; break;
        case 5: b.bq |= to; break;
        case 6: b.bk |= to; break;
        case 7: b.bp |= (to >> 8); break;
    }
}

inline void UnMoveBlack(const Move mv, chess &b){
    Bitboard f = (1ULL << (mv & 63));
    Bitboard to = (1ULL << ((mv>>6) & 63));

    // Move back piece
    switch ((mv >> 15) & 7){
        case 0: b.bp = (b.bp & ~to) | f; break;
        case 1: b.bn = (b.bn & ~to) | f; break;
        case 2: b.bb = (b.bb & ~to) | f; break;
        case 3: b.br = (b.br & ~to) | f; break;
        case 4: b.bq = (b.bq & ~to) | f; break;
        case 5:
            b.bk = (b.bk & ~to) | f;

            // Castle
            if (ctz(f) == 59){
                // Kingside
                if (ctz(to) == 57 || ctz(to) == 56) b.br = (b.br & ~(1ULL << 58)) | (1ULL << 56);
                //Queen
                else if (ctz(to) == 61 || ctz(to) == 63) b.br = (b.br & ~(1ULL << 60)) | (1ULL << 63);
            }
            break;
    }
     
    // Undo promotion
    switch (((mv >> 12) & 7)){
        case 1: b.bn &= ~to; break;
        case 2: b.bb &= ~to; break;
        case 3: b.br &= ~to; break;
        case 4: b.bq &= ~to; break;
    }

    // Undo capture
    switch (((mv >> 18) & 7)){
        case 1: b.wp |= to; break;
        case 2: b.wn |= to; break;
        case 3: b.wb |= to; break;
        case 4: b.wr |= to; break;
        case 5: b.wq |= to; break;
        case 6: b.wk |= to; break;
        case 7: b.wp |= (to << 8); break;
    }
}

inline void UnmovePiece(const Move mv, chess &b){
    b.WCastleKing = (mv >> 24) & 1;
    b.WCastleQueen = (mv >> 23) & 1;
    b.BCastleKing = (mv >> 22) & 1;
    b.BCastleQueen = (mv >> 21) & 1;
    b.ep = (int)((mv >> 25) & 127) - 1;

    if (b.turn) UnMoveBlack(mv, b);
    else UnMoveWhite(mv, b);

    b.turn = !b.turn;
    b.bpcs = b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs = b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces = b.bpcs|b.wpcs;
}

#endif