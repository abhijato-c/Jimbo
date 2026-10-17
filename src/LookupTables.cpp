void generate_rook_tables(){
    for(int i=0;i<64;++i){
        Bitboard piece=(1ULL << i);
        Bitboard side=0;
        Bitboard loc=piece;
        while ((loc<<8)!=0){
            side |= loc;
            loc <<= 8;
        }
        loc=piece;
        while ((loc>>8)!=0){
            side |= loc;
            loc >>= 8;
        }
        loc=piece;
        while ((loc&clear_h)!=0){
            side |= loc;
            loc >>= 1;
        }
        loc=piece;
        while ((loc&clear_a)!=0){
            side |= loc;
            loc <<= 1;
        }
        side&=~piece;
        rook_blockers[i]=side;
        for(int j=0;j<(1<<__builtin_popcountll(side));++j){
            Bitboard blockers=_pdep_u64(j,side);
            Bitboard moves=0;
            loc=piece<<8;
            if(loc!=0){
                moves|=loc;
                while (((loc&blockers)==0) && (loc!=0)){
                    loc <<= 8;
                    moves |= loc;
                }
            }
            loc=piece>>8;
            if(loc!=0){
                moves|=loc;
                while (((loc&blockers)==0) && (loc!=0)){
                    loc >>= 8;
                    moves |= loc;
                }
            }
            loc=piece<<1;
            if((loc&clear_h)!=0){
                moves|=loc;
                while (((loc&blockers)==0) && ((loc&clear_a)!=0)){
                    loc <<= 1;
                    moves |= loc;
                }
            }
            loc=piece>>1;
            if((loc&clear_a)!=0){
                moves|=loc;
                while (((loc&blockers)==0) && ((loc&clear_h)!=0)){
                    loc >>= 1;
                    moves |= loc;
                }
            }
            rook_moves_lkup[i][j]=moves;
        }
    }
}

void generate_bishop_tables(){
    for(int i=0;i<64;++i){
        Bitboard piece=(1ULL << i);
        Bitboard side=0;
        Bitboard loc=piece;
        while ((loc&clear_a&clear_8)!=0){
            side |= loc;
            loc <<= 9;
        }
        loc=piece;
        while ((loc&clear_h&clear_8)!=0){
            side |= loc;
            loc <<=7;
        }
        loc=piece;
        while ((loc&clear_a&clear_1)!=0){
            side |= loc;
            loc >>= 7;
        }
        loc=piece;
        while ((loc&clear_h&clear_1)!=0){
            side |= loc;
            loc >>= 9;
        }
        side&=~piece;
        bishop_blockers[i]=side;
        for(int j=0;j<(1<<__builtin_popcountll(side));++j){
            Bitboard blockers=_pdep_u64(j,side);
            Bitboard moves=0;
            loc=piece<<9;
            if((loc&clear_h)!=0){
                moves|=loc;
                while (((loc&blockers)==0) && (loc&clear_a)!=0){
                    loc <<= 9;
                    moves |= loc;
                }
            }
            loc=piece<<7;
            if((loc&clear_a)!=0){
                moves|=loc;
                while (((loc&blockers)==0) && (loc&clear_h)!=0){
                    loc <<= 7;
                    moves |= loc;
                }
            }
            loc=piece>>7;
            if((loc&clear_h)!=0){
                moves|=loc;
                while (((loc&blockers)==0) && ((loc&clear_a)!=0)){
                    loc >>= 7;
                    moves |= loc;
                }
            }
            loc=piece>>9;
            if((loc&clear_a)!=0){
                moves|=loc;
                while (((loc&blockers)==0) && ((loc&clear_h)!=0)){
                    loc >>= 9;
                    moves |= loc;
                }
            }
            bishop_moves_lkup[i][j]=moves;
        }
    }
}