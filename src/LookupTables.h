#ifndef LOOKUPTABLES_H
#define LOOKUPTABLES_H

#include "Defs.h"

// Generate Rook and Bishop Lookup Tables
inline Bitboard ComputeRookMoves(Bitboard piece, Bitboard blockers){
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

inline Bitboard ComputeBishopMoves(Bitboard piece, Bitboard blockers){
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

inline Bitboard ComputeRookBlockers(Bitboard piece){
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

inline Bitboard ComputeBishopBlockers(Bitboard piece){
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

#ifndef USE_BMI2
    inline unsigned int state = 1804289383;
    inline unsigned int get_random_U32() {
        unsigned int number = state;
        number ^= number << 13;
        number ^= number >> 17;
        number ^= number << 5;
        state = number;
        return number;
    }

    inline unsigned long long get_random_U64() {
        unsigned long long n1 = (unsigned long long)(get_random_U32()) & 0xFFFF;
        unsigned long long n2 = (unsigned long long)(get_random_U32()) & 0xFFFF;
        unsigned long long n3 = (unsigned long long)(get_random_U32()) & 0xFFFF;
        unsigned long long n4 = (unsigned long long)(get_random_U32()) & 0xFFFF;
        return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
    }

    // Generate a candidate magic number
    inline unsigned long long random_magic_candidate() {return get_random_U64() & get_random_U64() & get_random_U64();}

    // Returns the magic index
    inline Bitboard transform(Bitboard b, Bitboard magic, int bits) {return (b * magic) >> (64 - bits);}

    // Find magics
    inline void FindRookMagic(int sq){
        Bitboard mask = RookBlockers[sq];
        int bits = popcnt(mask);
        int num_permutations = 1 << bits;

        vector<Bitboard> blockers(num_permutations);
        vector<Bitboard> attacks(num_permutations);
        vector<Bitboard> used(num_permutations);

        // Pre-calculate all attacks for this square
        for (int i = 0; i < num_permutations; i++) {
            blockers[i] = pdep(i, mask);
            attacks[i] = ComputeRookMoves(1ULL << sq, blockers[i]);
        }

        
        // Search for a Magic Number
        for (int k = 0; k < 100000000; k++) {
            Bitboard magic = random_magic_candidate();
            
            // Skip magics that don't have enough bits to create entropy (Heuristic)
            if (popcnt((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

            // Verify the magic number
            fill(used.begin(), used.end(), 0ULL);
            bool fail = false;
            
            for (int i = 0; i < num_permutations; i++) {
                int idx = (int)transform(blockers[i], magic, bits);
                
                if (used[idx] == 0ULL) used[idx] = attacks[i]; // New pattern
                else if (used[idx] != attacks[i]) {
                    fail = true; // Collision with different attack set
                    break; 
                }
            }
            
            if (!fail) {
                // Save variables.
                RookMagics[sq] = magic;
                RookShifts[sq] = 64 - bits;
                return;
            }
        }
        cout << "Magic generation failed for rook square " << sq << endl;
    }

    inline void FindBishopMagic(int sq) {
        Bitboard mask = BishopBlockers[sq];
        int bits = popcnt(mask);
        int num_permutations = 1 << bits;
        
        vector<Bitboard> blockers(num_permutations);
        vector<Bitboard> attacks(num_permutations);
        vector<Bitboard> used(num_permutations);

        // 1. Pre-calculate all attacks for this square
        for (int i = 0; i < num_permutations; i++) {
            // Use software pdep to generate the blocker configuration
            blockers[i] = pdep(i, mask);
            attacks[i] = ComputeBishopMoves(1ULL << sq, blockers[i]);
        }

        // 2. Search for a Magic Number
        for (int k = 0; k < 100000000; k++) {
            Bitboard magic = random_magic_candidate();
            
            // Skip magics that don't have enough bits to create entropy (Heuristic)
            if (popcnt((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

            // Verify the magic number
            fill(used.begin(), used.end(), 0ULL);
            bool fail = false;
            
            for (int i = 0; i < num_permutations; i++) {
                int idx = (int)transform(blockers[i], magic, bits);
                
                if (used[idx] == 0ULL) used[idx] = attacks[i]; // New pattern
                else if (used[idx] != attacks[i]) {
                    fail = true; // Collision with different attack set
                    break; 
                }
            }
            
            if (!fail) {
                // Success! Save variables.
                BishopMagics[sq] = magic;
                BishopShifts[sq] = 64 - bits;
                return;
            }
        }
        cout << "Magic generation failed for bishop square " << sq << endl;
    }

#endif

inline void GenerateRookTables(){
    for(int i=0;i<64;++i){
        Bitboard piece=(1ULL << i);
        Bitboard Blockers = ComputeRookBlockers(piece);
        RookBlockers[i] = Blockers;

        #ifdef USE_BMI2
            // pext Bitboard
            for(int j=0; j<(1<<popcnt(Blockers)); ++j){
                Bitboard blockers = pdep(j, Blockers);
                RookMovesLookup[i][j] = ComputeRookMoves(piece, blockers);
            }
        #else
            // Magic Bitboard
            FindRookMagic(i); // Find magic for this square
            int bits = 64 - RookShifts[i];
            for(int j=0; j<(1<<bits); ++j){
                Bitboard blockers = pdep(j, RookBlockers[i]);
                int magic_idx = (int)((blockers * RookMagics[i]) >> RookShifts[i]);
                RookMovesLookup[i][magic_idx] = ComputeRookMoves(piece, blockers);
            }
        #endif
    }
}

inline void GenerateBishopTables(){
    for(int i=0;i<64;++i){
        Bitboard piece=(1ULL << i);
        Bitboard Blockers = ComputeBishopBlockers(piece);
        BishopBlockers[i] = Blockers;

        #ifdef USE_BMI2
            // pext Bitboard
            for(int j=0;j<(1<<popcnt(Blockers));++j){
                Bitboard blockers = pdep(j,Blockers);
                BishopMovesLookup[i][j] = ComputeBishopMoves(piece, blockers);
            }
        #else
            // Magic Bitboard
            FindBishopMagic(i);
            int bits = 64 - BishopShifts[i];
            for(int j=0; j<(1<<bits); ++j){
                Bitboard blockers = pdep(j, BishopBlockers[i]);
                int magic_idx = (int)((blockers * BishopMagics[i]) >> BishopShifts[i]);
                BishopMovesLookup[i][magic_idx] = ComputeBishopMoves(piece, blockers);
            }
        #endif
    }
}

inline void GenerateLookupTables(){
    GenerateRookTables();
    GenerateBishopTables();
}

#endif