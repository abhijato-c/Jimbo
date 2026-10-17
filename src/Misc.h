#ifndef MISC_H
#define MISC_H

#include "Defs.h"
#include <sstream>

inline int StaticEval(chess &b){
    ++nodes;
    if (b.turn)
        return ((popcnt(b.wb)*4) +
                (popcnt(b.wn)*3) +
                (popcnt(b.wp)) +
                (popcnt(b.wq)*9) +
                (popcnt(b.wk)*70) +
                (popcnt(b.wr)*5) -
                (popcnt(b.bb)*4) -
                (popcnt(b.bn)*3) -
                (popcnt(b.bp)) -
                (popcnt(b.bq)*9) -
                (popcnt(b.br)*5)-
                (popcnt(b.bk)*70));
    return ((popcnt(b.bb)*4) +
                (popcnt(b.bn)*3) +
                (popcnt(b.bp)) +
                (popcnt(b.bq)*9) +
                (popcnt(b.bk)*70) +
                (popcnt(b.br)*5) -
                (popcnt(b.wb)*4) -
                (popcnt(b.wn)*3) -
                (popcnt(b.wp)) -
                (popcnt(b.wq)*9) -
                (popcnt(b.wr)*5)-
                (popcnt(b.wk)*70));
}

inline string MoveToStr(const Move move){
    int from = (move & 63);
    int to = ((move>>6) & 63);
    int prom = ((move>>12) & 7);
    string MoveStr = "";

    MoveStr += (7 - from%8) + 'a'; // From file
    MoveStr += (from/8)     + '1'; // From rank
    MoveStr += (7 - to%8)   + 'a'; // To file
    MoveStr += (to/8)       + '1'; // To rank

    if (prom > 0){
        switch(prom){
            case 1:
                MoveStr += 'n';break;
            case 2:
                MoveStr += 'b';break;
            case 3:
                MoveStr += 'r';break;
            case 4:
                MoveStr += 'q';break;
        }
    }
    return MoveStr;
}

inline Move StrToMove(const string MoveStr){
    int FromFile = MoveStr[0] - 'a';
    int FromRank = MoveStr[1] - '1';
    int ToFile   = MoveStr[2] - 'a';
    int ToRank   = MoveStr[3] - '1';

    int from = (7 - FromFile) + (FromRank * 8);
    int to = (7 - ToFile) + (ToRank * 8);
    int prom = 0;

    if (MoveStr.length() == 5){
        switch(MoveStr[4]){
            case 'n':
                prom = 1; break;
            case 'b':
                prom = 2; break;
            case 'r':
                prom = 3; break;
            case 'q':
                prom = 4; break;
        }
    }
    return (prom << 12) | (to << 6) | (from);
}

inline void ParseFEN(string FullFEN, chess &b){
    // Split FEN
    vector<string> segments;
    size_t pos = 0;
    while ((pos = FullFEN.find(" ")) != string::npos) {
        segments.push_back(FullFEN.substr(0, pos));
        FullFEN.erase(0, pos + 1);
    }
    
    string fen = segments[0];
    string plr = segments[1];
    string CastleRights = segments[2];
    
    // Reset board
    b.wp=0ULL; b.wr=0ULL; b.wn=0ULL; b.wb=0ULL; b.wq=0ULL; b.wk=0ULL;
    b.bp=0ULL; b.br=0ULL; b.bn=0ULL; b.bb=0ULL; b.bq=0ULL; b.bk=0ULL;
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

    // Piece arrangement
    for (char c : fen) {
        if (isdigit(c)) {
            b.wp<<=c-'0'; b.wr<<=c-'0'; b.wn<<=c-'0'; b.wb<<=c-'0'; b.wq<<=c-'0'; b.wk<<=c-'0';
            b.bp<<=c-'0'; b.br<<=c-'0'; b.bn<<=c-'0'; b.bb<<=c-'0'; b.bq<<=c-'0'; b.bk<<=c-'0';
        }
        else if (c!='/'){
            b.wp<<=1; b.wr<<=1; b.wn<<=1; b.wb<<=1; b.wq<<=1; b.wk<<=1;
            b.bp<<=1; b.br<<=1; b.bn<<=1; b.bb<<=1; b.bq<<=1; b.bk<<=1;
            switch(c){
                case 'p':
                    b.bp+=1; break;
                case 'n':
                    b.bn+=1; break;
                case 'b':
                    b.bb+=1; break;
                case 'r':
                    b.br+=1; break;
                case 'q':
                    b.bq+=1; break;
                case 'P':
                    b.wp+=1; break;
                case 'N':
                    b.wn+=1; break;
                case 'B':
                    b.wb+=1; break;
                case 'R':
                    b.wr+=1; break;
                case 'Q':
                    b.wq+=1; break;
                case 'k':
                    b.bk+=1; break;
                case 'K':
                    b.wk+=1; break;
            }
        }
    }
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

#endif