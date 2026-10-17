#ifndef DEFS_H
#define DEFS_H

#include <iostream>
#include <vector>
#include <chrono>
#include <immintrin.h>
#include <bit>
#ifdef USE_BMI2
    #include <immintrin.h>
    #define pext(val, mask) _pext_u64(val, mask)
	#define pdep(val, mask) _pdep_u64(val, mask)
#endif

typedef unsigned long long int Bitboard;
typedef unsigned int Move;
typedef std::chrono::high_resolution_clock::time_point timept;
using namespace std;

constexpr int inf = 300000;

constexpr Bitboard clear_a=9187201950435737471ULL;
constexpr Bitboard clear_b=13816973012072644543ULL;
//constexpr Bitboard clear_c=16131858542891098079ULL;
//constexpr Bitboard clear_d=17289301308300324847ULL;
//constexpr Bitboard clear_e=17868022691004938231ULL;
//constexpr Bitboard clear_f=18157383382357244923ULL;
constexpr Bitboard clear_g=18302063728033398269ULL;
constexpr Bitboard clear_h=18374403900871474942ULL;

//constexpr Bitboard mask_a=9259542123273814144ULL;
//constexpr Bitboard mask_b=4629771061636907072ULL;
//constexpr Bitboard mask_c=2314885530818453536ULL;
//constexpr Bitboard mask_d=1157442765409226768ULL;
//constexpr Bitboard mask_e=578721382704613384ULL;
//constexpr Bitboard mask_f=289360691352306692ULL;
//constexpr Bitboard mask_g=144680345676153346ULL;
//constexpr Bitboard mask_h=72340172838076673ULL;

constexpr Bitboard clear_8=72057594037927935ULL;
//constexpr Bitboard clear_7=18374967954648334335ULL;
//constexpr Bitboard clear_6=18446463698244468735ULL;
//constexpr Bitboard clear_5=18446742978492891135ULL;
//constexpr Bitboard clear_4=18446744069431361535ULL;
//constexpr Bitboard clear_3=18446744073692839935ULL;
//constexpr Bitboard clear_2=18446744073709486335ULL;
constexpr Bitboard clear_1=18446744073709551360ULL;

//constexpr Bitboard mask_8=18374686479671623680ULL;
constexpr Bitboard mask_7=71776119061217280ULL;
//constexpr Bitboard mask_6=280375465082880ULL;
//constexpr Bitboard mask_5=1095216660480ULL;
//constexpr Bitboard mask_4=4278190080ULL;
//constexpr Bitboard mask_3=16711680ULL;
constexpr Bitboard mask_2=65280ULL;
//constexpr Bitboard mask_1=255ULL;

struct chess {
    Bitboard wp=0ULL; Bitboard wr=0ULL; Bitboard wn=0ULL; Bitboard wb=0ULL; Bitboard wq=0ULL; Bitboard wk=0ULL;
    Bitboard bp=0ULL; Bitboard br=0ULL; Bitboard bn=0ULL; Bitboard bb=0ULL; Bitboard bq=0ULL; Bitboard bk=0ULL;
    Bitboard bpcs=0; Bitboard wpcs=0; Bitboard pieces=0;
    bool WCastleKing = true; bool WCastleQueen = true;
    bool BCastleKing = true; bool BCastleQueen = true;
    bool turn = true;
};

struct MoveList {
    Move moves[256];
    int count = 0;
    
    inline void push_back(Move m) { moves[count++] = m; }
    
    // Functions required for foreach loop
    Move* begin() { return moves; }
    Move* end() { return moves + count; }
    const Move* begin() const { return moves; }
    const Move* end() const { return moves + count; }

    int size() const { return count; }
    Move operator[](int index) const { return moves[index]; }
    Move& operator[](int index) { return moves[index]; }
};

inline int nodes = 0;

inline Bitboard RookBlockers[64];
inline Bitboard RookMovesLookup[64][4096];
inline Bitboard BishopBlockers[64];
inline Bitboard BishopMovesLookup[64][512];

#ifndef USE_BMI2
    inline Bitboard RookMagics[64];
    inline int RookShifts[64];
    inline Bitboard BishopMagics[64];
    inline int BishopShifts[64];
    inline const int RookBits[64] = {
		12, 11, 11, 11, 11, 11, 11, 12,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		12, 11, 11, 11, 11, 11, 11, 12
	};
	inline const int BishopBits[64] = {
		6, 5, 5, 5, 5, 5, 5, 6,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		6, 5, 5, 5, 5, 5, 5, 6
	};
#endif

inline int ctz(const Bitboard b) { return std::countr_zero(b); }
inline int popcnt(const Bitboard b) { return std::popcount(b); }
#ifndef USE_BMI2
	inline Bitboard pdep(Bitboard val, Bitboard mask) {
		Bitboard res = 0;
		for (Bitboard m = mask; m; m &= (m - 1)) {
			Bitboard bit = m & -m; // Get lowest set bit of mask
			if (val & 1) res |= bit;
			val >>= 1;
		}
		return res;
	}
#endif

#endif