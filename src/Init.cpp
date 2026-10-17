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

using namespace std;
typedef unsigned long long int Bitboard;
typedef unsigned int Move;
typedef chrono::high_resolution_clock::time_point timept;

Bitboard rook_blockers[64];
Bitboard rook_moves_lkup[64][4096];
Bitboard bishop_blockers[64];
Bitboard bishop_moves_lkup[64][512];

// For magic bitboards
Bitboard RookMagics[64];
int RookShifts[64];
Bitboard BishopMagics[64];
int BishopShifts[64];
const int RookBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};
const int BishopBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

int nodes=0;

const Bitboard clear_a=9187201950435737471ULL;
const Bitboard clear_b=13816973012072644543ULL;
//const Bitboard clear_c=16131858542891098079ULL;
//const Bitboard clear_d=17289301308300324847ULL;
//const Bitboard clear_e=17868022691004938231ULL;
//const Bitboard clear_f=18157383382357244923ULL;
const Bitboard clear_g=18302063728033398269ULL;
const Bitboard clear_h=18374403900871474942ULL;

//const Bitboard mask_a=9259542123273814144ULL;
//const Bitboard mask_b=4629771061636907072ULL;
//const Bitboard mask_c=2314885530818453536ULL;
//const Bitboard mask_d=1157442765409226768ULL;
//const Bitboard mask_e=578721382704613384ULL;
//const Bitboard mask_f=289360691352306692ULL;
//const Bitboard mask_g=144680345676153346ULL;
//const Bitboard mask_h=72340172838076673ULL;

const Bitboard clear_8=72057594037927935ULL;
//const Bitboard clear_7=18374967954648334335ULL;
//const Bitboard clear_6=18446463698244468735ULL;
//const Bitboard clear_5=18446742978492891135ULL;
//const Bitboard clear_4=18446744069431361535ULL;
//const Bitboard clear_3=18446744073692839935ULL;
//const Bitboard clear_2=18446744073709486335ULL;
const Bitboard clear_1=18446744073709551360ULL;

//const Bitboard mask_8=18374686479671623680ULL;
const Bitboard mask_7=71776119061217280ULL;
//const Bitboard mask_6=280375465082880ULL;
//const Bitboard mask_5=1095216660480ULL;
//const Bitboard mask_4=4278190080ULL;
//const Bitboard mask_3=16711680ULL;
const Bitboard mask_2=65280ULL;
//const Bitboard mask_1=255ULL;

const int inf = 300000;

struct chess{
    Bitboard wp=0ULL; Bitboard wr=0ULL; Bitboard wn=0ULL; Bitboard wb=0ULL; Bitboard wq=0ULL; Bitboard wk=0ULL;
    Bitboard bp=0ULL; Bitboard br=0ULL; Bitboard bn=0ULL; Bitboard bb=0ULL; Bitboard bq=0ULL; Bitboard bk=0ULL;

    Bitboard bpcs=0; Bitboard wpcs=0; Bitboard pieces=0;

    bool WCastleKing = true; bool WCastleQueen = true;
    bool BCastleKing = true; bool BCastleQueen = true;

    bool turn=true;
};

inline int ctz(const Bitboard b){ return countr_zero(b); }
inline int popcnt(const Bitboard b){ return popcount(b); }

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