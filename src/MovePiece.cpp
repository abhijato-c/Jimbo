inline void move_piece_white(const Move &mv, chess &b){
    Bitboard t = ~(1ULL << ((mv>>6) & 63));
    b.bb &= t;
    b.bk &= t;
    b.bn &= t;
    b.bp &= t;
    b.bq &= t;
    b.br &= t;

    if(((mv>>12) & 7) != 0){
        b.wp &= ~(1ULL << (mv & 63));
        switch(((mv>>12) & 7)){
            case 1:
                b.wn |= ~t;
                break;
            case 2:
                b.wb |= ~t;
                break;
            case 3:
                b.wr |= ~t;
                break;
            case 4:
                b.wq |= ~t;
                break;
        }
    }
    else{
        Bitboard f = (1ULL << (mv & 63));
        if((b.wb&f)!=0)
            b.wb=(b.wb&(~f))|~t;
        else if((b.wr&f)!=0){
            b.wr=(b.wr&(~f))|~t;
            if ((mv & 63) == 0)
                b.WCastleKing = false;
            else if ((mv & 63)==7)
                b.WCastleQueen = false;
        }
        else if((b.wn&f)!=0)
            b.wn=(b.wn&(~f))|~t;
        else if((b.wp&f)!=0)
            b.wp=(b.wp&(~f))|~t;
        else if((b.wq&f)!=0)
            b.wq=(b.wq&(~f))|~t;
        else{
            b.wk=(b.wk&(~f))|~t;
            b.WCastleKing = false;
            b.WCastleQueen = false;
        }
    }
    b.turn=false;
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline void move_piece_black(const Move &mv, chess &b){
    Bitboard t = ~(1ULL << ((mv>>6) & 63));
    b.wb &= t;
    b.wk &= t;
    b.wn &= t;
    b.wp &= t;
    b.wq &= t;
    b.wr &= t;

    if(((mv>>12) & 7) != 0){
        b.bp &= ~(1ULL << (mv & 63));
        switch(((mv>>12) & 7)){
            case 1:
                b.bn |= ~t;
                break;
            case 2:
                b.bb |= ~t;
                break;
            case 3:
                b.br |= ~t;
                break;
            case 4:
                b.bq |= ~t;
                break;
        }
    }
    else{
        Bitboard f = (1ULL << (mv & 63));
        if((b.bb&f)!=0)
            b.bb=(b.bb&(~f))|~t;
        else if((b.br&f)!=0){
            b.br=(b.br&(~f))|~t;
            if ((mv & 63) == 56)
                b.BCastleKing = false;
            else if ((mv & 63) == 63)
                b.BCastleQueen = false;
        }
        else if((b.bn&f)!=0)
            b.bn=(b.bn&(~f))|~t;
        else if((b.bp&f)!=0)
            b.bp=(b.bp&(~f))|~t;
        else if((b.bq&f)!=0)
            b.bq=(b.bq&(~f))|~t;
        else{
            b.bk=(b.bk&(~f))|~t;
            b.BCastleKing = false;
            b.BCastleQueen = false;
        }
    }
    b.turn=true;
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline void move_piece(const Move &mv, chess &b){
    switch(b.turn){
        case true:
            return move_piece_white(mv,b);
        case false:
            return move_piece_black(mv,b);
    }
}