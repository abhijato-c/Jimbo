#ifndef MISC_H
#define MISC_H

#include "Defs.h"
#include <sstream>

inline int StaticEval(const chess &b){
    if (b.turn)
        return ((popcnt(b.wb)*4) + (popcnt(b.wn)*3) + (popcnt(b.wp)) + (popcnt(b.wq)*9) + (popcnt(b.wr)*5) -
                (popcnt(b.bb)*4) - (popcnt(b.bn)*3) - (popcnt(b.bp)) - (popcnt(b.bq)*9) - (popcnt(b.br)*5));
    return ((popcnt(b.bb)*4) + (popcnt(b.bn)*3) + (popcnt(b.bp)) + (popcnt(b.bq)*9) + (popcnt(b.br)*5) -
            (popcnt(b.wb)*4) - (popcnt(b.wn)*3) - (popcnt(b.wp)) - (popcnt(b.wq)*9) - (popcnt(b.wr)*5));
}

inline bool IsAttacked(const chess &b, int sq, bool byWhite){
    const Bitboard sqBB = 1ULL << sq;

    const Bitboard KnightAtt = KnightAttacks[sq];
    const Bitboard KingAtt = KingAttacks[sq];

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
        if (b.wp & (((sqBB & clear_h) >> 7) | ((sqBB & clear_a) >> 9))) return true;
        if (b.wn & KnightAtt)               return true;
        if ((b.wb | b.wq) & diag)    return true;
        if ((b.wr | b.wq) & orth)    return true;
        if (b.wk & KingAtt)         return true;
    } 
    else {
        if (b.bp & (((sqBB & clear_a) << 7) | ((sqBB & clear_h) << 9))) return true;
        if (b.bn & KnightAtt)               return true;
        if ((b.bb | b.bq) & diag)    return true;
        if ((b.br | b.bq) & orth)    return true;
        if (b.bk & KingAtt)         return true;
    }
    return false;
}

inline string MoveToStr(const Move move){
    int from = (move & 63);
    int to = ((move >> 6) & 63);
    int prom = ((move >> 12) & 7);
    string MoveStr = "";

    MoveStr += (from % 8) + 'a'; // From file
    MoveStr += (from / 8) + '1'; // From rank
    MoveStr += (to % 8)   + 'a'; // To file
    MoveStr += (to / 8)   + '1'; // To rank

    if (prom > 0){
        switch(prom){
            case 2: MoveStr += 'n'; break;
            case 3: MoveStr += 'b'; break;
            case 4: MoveStr += 'r'; break;
            case 5: MoveStr += 'q'; break;
        }
    }

    if (MoveStr == "a1a1") return "0000";
    return MoveStr;
}

inline Move StrToMove(const string MoveStr, chess& b){
    int FromFile = MoveStr[0] - 'a';
    int FromRank = MoveStr[1] - '1';
    int ToFile   = MoveStr[2] - 'a';
    int ToRank   = MoveStr[3] - '1';

    uint32_t from = FromFile + (FromRank * 8);
    uint32_t to = ToFile + (ToRank * 8);
    uint32_t prom = 0;
    uint32_t fromPc = 0;
    uint32_t capPc = 0;

    // Handle Promoted Piece
    if (MoveStr.length() == 5){
        switch(MoveStr[4]){
            case 'n': prom = 2; break;
            case 'b': prom = 3; break;
            case 'r': prom = 4; break;
            case 'q': prom = 5; break;
        }
    }

    // Identify from piece
    if ((b.wp | b.bp) & (1ULL << from)) fromPc = 1;
    else if ((b.wn | b.bn) & (1ULL << from)) fromPc = 2;
    else if ((b.wb | b.bb) & (1ULL << from)) fromPc = 3;
    else if ((b.wr | b.br) & (1ULL << from)) fromPc = 4;
    else if ((b.wq | b.bq) & (1ULL << from)) fromPc = 5;
    else if ((b.wk | b.bk) & (1ULL << from)) fromPc = 6;

    // Identify captured piece
    if ((b.wp | b.bp) & (1ULL << to)) capPc = 1;
    else if ((b.wn | b.bn) & (1ULL << to)) capPc = 2;
    else if ((b.wb | b.bb) & (1ULL << to)) capPc = 3;
    else if ((b.wr | b.br) & (1ULL << to)) capPc = 4;
    else if ((b.wq | b.bq) & (1ULL << to)) capPc = 5;
    else if ((b.wk | b.bk) & (1ULL << to)) capPc = 6;

    // En passant detection
    if (fromPc == 1 && FromFile != ToFile && capPc == 0)  capPc = 7;
    
    return (capPc << 18) | (fromPc << 15) | (prom << 12) | (to << 6) | from;
}

inline void ParseFEN(string FullFEN, chess &b){
    // Split FEN
    vector<string> segments = {};
    string token;
    istringstream stream(FullFEN);
    while (stream >> token) {segments.push_back(token);}
    if (segments.size() < 4) return;
    
    string fen = segments[0];
    string plr = segments[1];
    string CastleRights = segments[2];
    string epSquare = segments[3];
    
    // Reset board
    b.wp=0ULL; b.wr=0ULL; b.wn=0ULL; b.wb=0ULL; b.wq=0ULL; b.wk=0ULL;
    b.bp=0ULL; b.br=0ULL; b.bn=0ULL; b.bb=0ULL; b.bq=0ULL; b.bk=0ULL;
    b.ep = -1;
    b.WCastleKing = false; b.WCastleQueen = false; b.BCastleKing = false; b.BCastleQueen = false;

    // Set turn
    if (plr == "w") b.turn = true;
    else b.turn = false;

    // Castling rights
    for (char c : CastleRights) {
        if (c == 'K') b.WCastleKing = true;
        else if (c == 'Q') b.WCastleQueen = true;
        else if (c == 'k') b.BCastleKing = true;
        else if (c == 'q') b.BCastleQueen = true;
    }

    // Ep square
    if (epSquare != "-")
        b.ep = epSquare[0] - 'a';

    // Piece arrangement (a1 = 0, b1 = 1, ... h1 = 7, a2 = 8, ... h8 = 63)
    int rank = 7, file = 0;
    for (char c : fen) {
        if (c == '/')        { --rank; file = 0; }
        else if (isdigit(c)) { file += c - '0'; }
        else {
            const Bitboard sq = 1ULL << (rank * 8 + file);
            ++file;
            switch(c){
                case 'p': b.bp |= sq; break;
                case 'n': b.bn |= sq; break;
                case 'b': b.bb |= sq; break;
                case 'r': b.br |= sq; break;
                case 'q': b.bq |= sq; break;
                case 'k': b.bk |= sq; break;
                case 'P': b.wp |= sq; break;
                case 'N': b.wn |= sq; break;
                case 'B': b.wb |= sq; break;
                case 'R': b.wr |= sq; break;
                case 'Q': b.wq |= sq; break;
                case 'K': b.wk |= sq; break;
            }
        }
    }
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

#endif