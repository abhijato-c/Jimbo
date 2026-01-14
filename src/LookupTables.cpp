// Magic Bitboard
// Simple Xorshift pseudo-random number generator
unsigned int state = 1804289383;
unsigned int get_random_U32() {
    unsigned int number = state;
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    state = number;
    return number;
}

unsigned long long get_random_U64() {
    unsigned long long n1 = (unsigned long long)(get_random_U32()) & 0xFFFF;
    unsigned long long n2 = (unsigned long long)(get_random_U32()) & 0xFFFF;
    unsigned long long n3 = (unsigned long long)(get_random_U32()) & 0xFFFF;
    unsigned long long n4 = (unsigned long long)(get_random_U32()) & 0xFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Generate a candidate magic number
unsigned long long random_magic_candidate() {return get_random_U64() & get_random_U64() & get_random_U64();}

// Returns the magic index
Bitboard transform(Bitboard b, Bitboard magic, int bits) {return (b * magic) >> (64 - bits);}

// Find magics here

// Generate Rook and Bishop Lookup Tables
Bitboard pdep(Bitboard val, Bitboard mask) {
    //TODO: Optimize using BMI2 instruction _pdep_u64 if available (_pdep_u64)
    Bitboard res = 0;
    for (Bitboard m = mask; m; m &= (m - 1)) {
        Bitboard bit = m & -m; // Get lowest set bit of mask
        if (val & 1) res |= bit;
        val >>= 1;
    }
    return res;
}

Bitboard ComputeRookMoves(Bitboard piece, Bitboard blockers){
    Bitboard moves=0;
    Bitboard loc = piece<<8;
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
    return moves;
}

Bitboard ComputeBishopMoves(Bitboard piece, Bitboard blockers){
    Bitboard moves=0;
    Bitboard loc=piece<<9;
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
    return moves;
}

Bitboard ComputeRookBlockers(Bitboard piece){
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
    return side;
}

Bitboard ComputeBishopBlockers(Bitboard piece){
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
    return side;
}

void GenerateRookTables(){
    for(int i=0;i<64;++i){
        Bitboard piece=(1ULL << i);
        Bitboard Blockers = ComputeRookBlockers(piece);
        rook_blockers[i] = Blockers;

        for(int j=0; j<(1<<popcnt(Blockers)); ++j){
            Bitboard blockers = pdep(j, Blockers);
            rook_moves_lkup[i][j] = ComputeRookMoves(piece, blockers);
        }
    }
}

void GenerateBishopTables(){
    for(int i=0;i<64;++i){
        Bitboard piece=(1ULL << i);
        Bitboard Blockers = ComputeBishopBlockers(piece);
        bishop_blockers[i] = Blockers;

        for(int j=0;j<(1<<popcnt(Blockers));++j){
            Bitboard blockers = pdep(j,Blockers);
            bishop_moves_lkup[i][j] = ComputeBishopMoves(piece, blockers);
        }
    }
}

void GenerateLookupTables(){
    GenerateRookTables();
    GenerateBishopTables();
}