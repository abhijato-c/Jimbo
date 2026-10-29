#ifndef MOVEPIECE_H
#define MOVEPIECE_H

#include "Defs.h"
#include "Misc.h"

inline void UnmovePiece(Board &b, Move &m) {
    int FromSquare = m & 63;
    int ToSquare = (m >> 6) & 63;
    int Type = (m >> 15) & 7;
    int Promote = (m >> 12) & 7;
    
    b.turn = b.turn ^ 1;
    b.EpSquare = ((m >> 25) & 15) - 1;
    b.CastleRights = (m >> 21) & 15;
    
    if (Type == KING && abs(ToSquare - FromSquare) == 2) {
        int RookFrom = ToSquare > FromSquare ? FromSquare + 3 : FromSquare - 4;
        int RookTo = (FromSquare + ToSquare) / 2;
        Take(b, b.turn, ROOK, RookTo); 
        Put(b, b.turn, ROOK, RookFrom);
    }
    
    Take(b, b.turn, Promote ? Promote : Type, ToSquare);
    Put(b, b.turn, Type, FromSquare);
    
    int Capture = (m >> 18) & 7;
    if (Capture)
        Put(b, b.turn ^ 1, Capture == EP ? PAWN : Capture, Capture == EP ? ToSquare + ((b.turn == WHITE) ? -8 : 8) : ToSquare);    
}

inline bool MovePiece(Board &b, Move &m) {
    int FromSquare = m & 63;
    int ToSquare = (m >> 6) & 63;
    int Type = (m >> 15) & 7;
    int Promote = (m >> 12) & 7;
    int Capture = ((m >> 18) & 7) == EP ? EP : b.OccupancyType[ToSquare];
    m = (m & ~(7u << 18)) | Capture << 18 | (b.EpSquare + 1) << 25 | b.CastleRights << 21;

    if (Capture)
        Take(b, b.turn ^ 1, Capture == EP ? PAWN : Capture, Capture == EP ? ToSquare + ((b.turn == WHITE) ? -8 : 8) : ToSquare);
    
    Take(b, b.turn, Type, FromSquare);
    Put(b, b.turn, Promote ? Promote : Type, ToSquare);

    // Castling
    if (Type == KING && abs(ToSquare - FromSquare) == 2) {
        int RookFrom = ToSquare > FromSquare ? FromSquare + 3 : FromSquare - 4;
        int RookTo = (FromSquare + ToSquare) / 2;
        Take(b, b.turn, ROOK, RookFrom); 
        Put(b, b.turn, ROOK, RookTo);
    }
    b.CastleRights &= CASTLE_MASK[FromSquare] & CASTLE_MASK[ToSquare];
    b.EpSquare = (Type == PAWN && abs(ToSquare - FromSquare) == 16) ? (FromSquare & 7) : -1;
    b.turn = b.turn ^ 1;

    if (IsAttacked(b, ctz(b.PieceBitboards[b.turn ^ 1][KING]), b.turn)) { 
        UnmovePiece(b, m); 
        return false; 
    }
    return true;
}

#endif