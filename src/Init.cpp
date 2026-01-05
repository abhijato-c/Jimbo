#include <iostream>
#include <vector>
#include <chrono>
#include <immintrin.h>

using namespace std;
typedef unsigned long long int Bitboard;
typedef unsigned int Move;
typedef chrono::_V2::system_clock::time_point timept;

Bitboard rook_blockers[64];
Bitboard rook_moves_lkup[64][4096];
Bitboard bishop_blockers[64];
Bitboard bishop_moves_lkup[64][512];
int nodes=0;
int mdepth=7;

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

struct chess{
    Bitboard wp=0ULL;
    Bitboard wr=0ULL;
    Bitboard wn=0ULL;
    Bitboard wb=0ULL;
    Bitboard wq=0ULL;
    Bitboard wk=0ULL;

    Bitboard bp=0ULL;
    Bitboard br=0ULL;
    Bitboard bn=0ULL;
    Bitboard bb=0ULL;
    Bitboard bq=0ULL;
    Bitboard bk=0ULL;

    Bitboard bpcs=0;
    Bitboard wpcs=0;
    Bitboard pieces=0;

    bool WCastleKing = true;
    bool WCastleQueen = true;
    bool BCastleKing = true;
    bool BCastleQueen = true;

    bool turn=true;
};

inline int ctz(const Bitboard &pos){
    return __builtin_ctzll(pos);
}