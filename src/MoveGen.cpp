inline void WKmoves(chess &b, vector<Move> &Moves){
    int i = ctz(b.wk);
    
    if ((((1ULL << i) & clear_h) << 7) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) << 7) << 6 | i);
    if (((1ULL << i) << 8) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) << 8) << 6 | i);
    if ((((1ULL << i) & clear_a) << 9) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) << 9) << 6 | i);
    if ((((1ULL << i) & clear_a) << 1) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) << 1) << 6 | i);
    if ((((1ULL << i) & clear_a) >> 7) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) >> 7) << 6 | i);
    if (((1ULL << i) >> 8) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) >> 8) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 9) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) >> 9) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 1) &~ b.wpcs)
        Moves.push_back(ctz((1ULL << i) >> 1) << 6 | i);
}

inline void BKmoves(chess &b, vector<Move> &Moves){
    int i = ctz(b.bk);

    if ((((1ULL << i) & clear_h) << 7) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) << 7) << 6 | i);
    if (((1ULL << i) << 8) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) << 8) << 6 | i);
    if ((((1ULL << i) & clear_a) << 9) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) << 9) << 6 | i);
    if ((((1ULL << i) & clear_a) << 1) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) << 1) << 6 | i);
    if ((((1ULL << i) & clear_a) >> 7) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) >> 7) << 6 | i);
    if (((1ULL << i) >> 8) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) >> 8) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 9) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) >> 9) << 6 | i);
    if ((((1ULL << i) & clear_h) >> 1) &~ b.bpcs)
        Moves.push_back(ctz((1ULL << i) >> 1) << 6 | i);
}

inline void WNmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.wn;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        if ((((1ULL << i) & clear_a & clear_b) >> 6) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) >> 6) << 6 | i);
        if ((((1ULL << i) & clear_a) >> 15) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) >> 15) << 6 | i);
        if ((((1ULL << i) & clear_h) >> 17) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) >> 17) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) >> 10) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) >> 10) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) << 6) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) << 6) << 6 | i);
        if ((((1ULL << i) & clear_h) << 15) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) << 15) << 6 | i);
        if ((((1ULL << i) & clear_a) << 17) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) << 17) << 6 | i);
        if ((((1ULL << i) & clear_a & clear_b) << 17) &~ b.wpcs)
            Moves.push_back(ctz((1ULL << i) << 17) << 6 | i);
    }
}

inline void BNmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.bn;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        if ((((1ULL << i) & clear_a & clear_b) >> 6) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) >> 6) << 6 | i);
        if ((((1ULL << i) & clear_a) >> 15) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) >> 15) << 6 | i);
        if ((((1ULL << i) & clear_h) >> 17) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) >> 17) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) >> 10) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) >> 10) << 6 | i);
        if ((((1ULL << i) & clear_h & clear_g) << 6) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) << 6) << 6 | i);
        if ((((1ULL << i) & clear_h) << 15) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) << 15) << 6 | i);
        if ((((1ULL << i) & clear_a) << 17) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) << 17) << 6 | i);
        if ((((1ULL << i) & clear_a & clear_b) << 17) &~ b.bpcs)
            Moves.push_back(ctz((1ULL << i) << 17) << 6 | i);
    }
}

inline void WPmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.wp;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        if (((1ULL << i) << 8) &~ b.pieces)
            Moves.push_back(ctz((1ULL << i) << 8) << 6 | i);
        if ((((((1ULL << i) & mask_2) << 8) &~ b.pieces) << 8) &~ b.pieces)
            Moves.push_back(ctz((1ULL << i) << 16) << 6 | i);
        if (((1ULL << i) << 7) & b.bpcs & clear_a)
            Moves.push_back(ctz((1ULL << i) << 7) << 6 | i);
        if (((1ULL << i) << 9) & b.bpcs & clear_h)
            Moves.push_back(ctz((1ULL << i) << 9) << 6 | i);
    }
}

inline void BPmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.bp;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        if (((1ULL << i) >> 8) &~ b.pieces)
            Moves.push_back(ctz((1ULL << i) >> 8) << 6 | i);
        if ((((((1ULL << i) & mask_7) >> 8) &~ b.pieces) >> 8) &~ b.pieces)
            Moves.push_back(ctz((1ULL << i) >> 16) << 6 | i);
        if (((1ULL << i) >> 7) & b.wpcs & clear_h)
            Moves.push_back(ctz((1ULL << i) >> 7) << 6 | i);
        if (((1ULL << i) >> 9) & b.wpcs & clear_a)
            Moves.push_back(ctz((1ULL << i) >> 9) << 6 | i);
    }
}

inline void WBmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.wb;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        Bitboard it = (bishop_moves_lkup[i][_pext_u64(b.pieces,bishop_blockers[i])] &~ b.wpcs);
        int j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }
    }
}

inline void BBmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.bb;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        Bitboard it = (bishop_moves_lkup[i][_pext_u64(b.pieces,bishop_blockers[i])] &~ b.bpcs);
        int j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }
    }
}

inline void WRmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.wr;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        Bitboard it = (rook_moves_lkup[i][_pext_u64(b.pieces,rook_blockers[i])] &~ b.wpcs);
        int j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }
    }
}

inline void BRmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.br;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        Bitboard it = (rook_moves_lkup[i][_pext_u64(b.pieces,rook_blockers[i])] &~ b.bpcs);
        int j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }
    }
}

inline void WQmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.wq;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        Bitboard it = (rook_moves_lkup[i][_pext_u64(b.pieces,rook_blockers[i])] &~ b.wpcs);
        int j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }

        it = (bishop_moves_lkup[i][_pext_u64(b.pieces,bishop_blockers[i])] &~ b.wpcs);
        j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }
    }
}

inline void BQmoves(chess &b, vector<Move> &Moves){
    Bitboard iter = b.bq;
    int i = -1;
    while(iter!=0){
        i += ctz(iter) + 1;
        iter >>= ctz(iter);
        iter >>= 1;

        Bitboard it = (rook_moves_lkup[i][_pext_u64(b.pieces,rook_blockers[i])] &~ b.bpcs);
        int j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }

        it = (bishop_moves_lkup[i][_pext_u64(b.pieces,bishop_blockers[i])] &~ b.bpcs);
        j = -1;
        while(it!=0){
            j += ctz(it) + 1;
            it >>= ctz(it);
            it >>= 1;

            Moves.push_back((j << 6) | i);
        }
    }
}

inline vector<Move> moves_white(chess &b){
    vector<Move> moves = {};

    WKmoves(b,moves);
    WNmoves(b,moves);
    WPmoves(b,moves);
    WBmoves(b,moves);
    WRmoves(b,moves);
    WQmoves(b,moves);

    Bitboard v=b.wp&mask_7;
    int i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i+8))&b.pieces)==0){
            moves.push_back((1 << 12) | ((i+8) << 6) | i);
            moves.push_back((2 << 12) | ((i+8) << 6) | i);
            moves.push_back((3 << 12) | ((i+8) << 6) | i);
            moves.push_back((4 << 12) | ((i+8) << 6) | i);
        }
        v>>=1;
    }
    v=b.wp&mask_7&clear_h;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i+7))&b.bpcs)!=0){
            moves.push_back((1 << 12) | ((i+7) << 6) | i);
            moves.push_back((2 << 12) | ((i+7) << 6) | i);
            moves.push_back((3 << 12) | ((i+7) << 6) | i);
            moves.push_back((4 << 12) | ((i+7) << 6) | i);
        }
        v>>=1;
    }
    v=b.wp&mask_7&clear_a;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i+9))&b.bpcs)!=0){
            moves.push_back((1 << 12) | ((i+9) << 6) | i);
            moves.push_back((2 << 12) | ((i+9) << 6) | i);
            moves.push_back((3 << 12) | ((i+9) << 6) | i);
            moves.push_back((4 << 12) | ((i+9) << 6) | i);
        }
        v>>=1;
    }
    return moves;
}

inline vector<Move> moves_black(chess &b){
    vector<Move> moves = {};

    BKmoves(b,moves);
    BNmoves(b,moves);
    BPmoves(b,moves);
    BBmoves(b,moves);
    BRmoves(b,moves);
    BQmoves(b,moves);

    Bitboard v=b.bp&mask_2;
    int i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i-8))&b.pieces)==0){
            moves.push_back((1 << 12) | ((i-8) << 6) | i);
            moves.push_back((2 << 12) | ((i-8) << 6) | i);
            moves.push_back((3 << 12) | ((i-8) << 6) | i);
            moves.push_back((4 << 12) | ((i-8) << 6) | i);
        }
        v>>=1;
    }
    v=b.bp&mask_2&clear_a;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i-7))&b.wpcs)!=0){
            moves.push_back((1 << 12) | ((i-7) << 6) | i);
            moves.push_back((2 << 12) | ((i-7) << 6) | i);
            moves.push_back((3 << 12) | ((i-7) << 6) | i);
            moves.push_back((4 << 12) | ((i-7) << 6) | i);
        }
        v>>=1;
    }
    v=b.bp&mask_2&clear_h;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i-9))&b.wpcs)!=0){
            moves.push_back((1 << 12) | ((i-9) << 6) | i);
            moves.push_back((2 << 12) | ((i-9) << 6) | i);
            moves.push_back((3 << 12) | ((i-9) << 6) | i);
            moves.push_back((4 << 12) | ((i-9) << 6) | i);
        }
        v>>=1;
    }
    return moves;
}

inline vector<Move> pl_moves(chess &b){
    switch (b.turn){
        case true:
            return moves_white(b);
        case false:
            return moves_black(b);
    }
}