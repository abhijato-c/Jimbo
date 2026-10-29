#ifndef MISC_H
#define MISC_H

#include "Defs.h"
#include <sstream>

inline int StaticEval(const Board &b){
    int phase = clamp(b.Phase, 0, 24);
    int score = (phase * b.ScoreMg) + ((24 - phase) * b.ScoreEg);
    return (b.turn == WHITE) ? score : -score;
}

inline void Put(Board &b, int Color, int Type, int Square) {
    Bitboard bit = 1ULL << Square;

    b.PieceBitboards[Color][Type] ^= bit; 
    b.OccupancyColor[Color] ^= bit; 
    b.OccupancyAll ^= bit; 
    b.OccupancyType[Square] = Type;

    b.ScoreMg += PsqtMg[Color][Type][Square]; 
    b.ScoreEg += PsqtEg[Color][Type][Square]; 
    b.Phase   += PhaseValues[Type];
}

inline void Take(Board &b, int Color, int Type, int Square) {
    Bitboard bit = 1ULL << Square;

    b.PieceBitboards[Color][Type] ^= bit; 
    b.OccupancyColor[Color] ^= bit; 
    b.OccupancyAll ^= bit; 
    b.OccupancyType[Square] = 0;

    b.ScoreMg -= PsqtMg[Color][Type][Square]; 
    b.ScoreEg -= PsqtEg[Color][Type][Square]; 
    b.Phase   -= PhaseValues[Type];
}

inline bool IsAttacked(const Board &b, int sq, int AttackedByColor){
    const Bitboard sqBB = 1ULL << sq;

    const Bitboard KnightAtt = KnightAttacks[sq];
    const Bitboard KingAtt = KingAttacks[sq];
    Bitboard RookAtt;
    Bitboard BishopAtt;
    Bitboard QueenAtt;

    #ifdef USE_BMI2
        BishopAtt = BishopMovesLookup[sq][pext(b.OccupancyAll, BishopBlockers[sq])];
        RookAtt = RookMovesLookup[sq][pext(b.OccupancyAll, RookBlockers[sq])];
        QueenAtt = BishopAtt | RookAtt;
    #else
        BishopAtt = BishopMovesLookup[sq][((b.OccupancyAll & BishopBlockers[sq]) * BishopMagics[sq]) >> BishopShifts[sq]];
        RookAtt = RookMovesLookup[sq][((b.OccupancyAll & RookBlockers[sq])   * RookMagics[sq])   >> RookShifts[sq]];
        QueenAtt = BishopAtt | RookAtt;
    #endif
 
    if (AttackedByColor == WHITE){
        if (b.PieceBitboards[WHITE][PAWN] & (((sqBB & clear_h) >> 7) | ((sqBB & clear_a) >> 9))) return true;
        if (b.PieceBitboards[WHITE][KNIGHT] & KnightAtt) return true;
        if (b.PieceBitboards[WHITE][BISHOP] & BishopAtt) return true;
        if (b.PieceBitboards[WHITE][ROOK]   & RookAtt)   return true;
        if (b.PieceBitboards[WHITE][QUEEN]  & QueenAtt)  return true;
        if (b.PieceBitboards[WHITE][KING]   & KingAtt)   return true;
    } 
    else {
        if (b.PieceBitboards[BLACK][PAWN] & (((sqBB & clear_a) << 7) | ((sqBB & clear_h) << 9))) return true;
        if (b.PieceBitboards[BLACK][KNIGHT] & KnightAtt) return true;
        if (b.PieceBitboards[BLACK][BISHOP] & BishopAtt) return true;
        if (b.PieceBitboards[BLACK][ROOK]   & RookAtt)   return true;
        if (b.PieceBitboards[BLACK][QUEEN]  & QueenAtt)  return true;
        if (b.PieceBitboards[BLACK][KING]   & KingAtt)   return true;
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

inline Move StrToMove(const string MoveStr, Board &b){
    int FromFile = MoveStr[0] - 'a';
    int FromRank = MoveStr[1] - '1';
    int ToFile   = MoveStr[2] - 'a';
    int ToRank   = MoveStr[3] - '1';

    int from = FromFile + (FromRank * 8);
    int to = ToFile + (ToRank * 8);
    int PromoteType = 0;
    int FromType = 0;
    int CaptureType = 0;

    // Handle Promoted Piece
    if (MoveStr.length() == 5){
        switch(MoveStr[4]){
            case 'n': PromoteType = 2; break;
            case 'b': PromoteType = 3; break;
            case 'r': PromoteType = 4; break;
            case 'q': PromoteType = 5; break;
        }
    }

    // Identify from piece
    FromType = b.OccupancyType[from];

    // Identify captured piece
    CaptureType = b.OccupancyType[to];
    if (FromType == 1 && FromFile != ToFile && CaptureType == 0)  CaptureType = 7;
    
    return (CaptureType << 18) | (FromType << 15) | (PromoteType << 12) | (to << 6) | from;
}

inline void ParseFEN(string FullFEN, Board &b){
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
    for (int pt = 0; pt < 7; ++pt) {b.PieceBitboards[0][pt] = 0ULL; b.PieceBitboards[1][pt] = 0ULL; }
    for (int sq = 0; sq < 64; ++sq) b.OccupancyType[sq] = 0;
    b.OccupancyColor[0] = 0ULL; b.OccupancyColor[1] = 0ULL; b.OccupancyAll = 0ULL;
    b.ScoreMg = 0; b.ScoreEg = 0; b.Phase = 0;
    b.EpSquare = -1; b.CastleRights = 0;

    // Set turn
    if (plr == "w") b.turn = WHITE;
    else b.turn = BLACK;

    // Castling rights
    for (char c : CastleRights) {
        if (c == 'K')      b.CastleRights |= 8;
        else if (c == 'Q') b.CastleRights |= 4;
        else if (c == 'k') b.CastleRights |= 2;
        else if (c == 'q') b.CastleRights |= 1;
    }

    // Ep square
    if (epSquare != "-")
        b.EpSquare = epSquare[0] - 'a';

    // Piece arrangement (a1 = 0, b1 = 1, ... h1 = 7, a2 = 8, ... h8 = 63)
    int rank = 7, file = 0;
    for (char c : fen) {
        if (c == '/')        { --rank; file = 0; }
        else if (isdigit(c)) { file += c - '0'; }
        else {
            Bitboard sq = 1ULL << (rank * 8 + file);
            int ind = rank * 8 + file;
            ++file;
            switch(c){
                case 'p': Put(b, BLACK, PAWN, ind);   break;
                case 'n': Put(b, BLACK, KNIGHT, ind); break;
                case 'b': Put(b, BLACK, BISHOP, ind); break;
                case 'r': Put(b, BLACK, ROOK, ind);   break;
                case 'q': Put(b, BLACK, QUEEN, ind);  break;
                case 'k': Put(b, BLACK, KING, ind);   break;
                case 'P': Put(b, WHITE, PAWN, ind);   break;
                case 'N': Put(b, WHITE, KNIGHT, ind); break;
                case 'B': Put(b, WHITE, BISHOP, ind); break;
                case 'R': Put(b, WHITE, ROOK, ind);   break;
                case 'Q': Put(b, WHITE, QUEEN, ind);  break;
                case 'K': Put(b, WHITE, KING, ind);   break;
            }
            
        }
    }
}

#endif