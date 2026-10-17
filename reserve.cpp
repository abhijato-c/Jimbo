#include <iostream>
#include <vector>
#include <chrono>
#include <immintrin.h>

using namespace std;
typedef unsigned long long int bitboard;
typedef std::chrono::_V2::system_clock::time_point timept;

bitboard rook_blockers[64];
bitboard rook_moves_lkup[64][4096];
bitboard bishop_blockers[64];
bitboard bishop_moves_lkup[64][512];
int nodes=0;
int mdepth=7;

const bitboard clear_a=9187201950435737471ULL;
const bitboard clear_b=13816973012072644543ULL;
//const bitboard clear_c=16131858542891098079ULL;
//const bitboard clear_d=17289301308300324847ULL;
//const bitboard clear_e=17868022691004938231ULL;
//const bitboard clear_f=18157383382357244923ULL;
const bitboard clear_g=18302063728033398269ULL;
const bitboard clear_h=18374403900871474942ULL;

//const bitboard mask_a=9259542123273814144ULL;
//const bitboard mask_b=4629771061636907072ULL;
//const bitboard mask_c=2314885530818453536ULL;
//const bitboard mask_d=1157442765409226768ULL;
//const bitboard mask_e=578721382704613384ULL;
//const bitboard mask_f=289360691352306692ULL;
//const bitboard mask_g=144680345676153346ULL;
//const bitboard mask_h=72340172838076673ULL;

const bitboard clear_8=72057594037927935ULL;
//const bitboard clear_7=18374967954648334335ULL;
//const bitboard clear_6=18446463698244468735ULL;
//const bitboard clear_5=18446742978492891135ULL;
//const bitboard clear_4=18446744069431361535ULL;
//const bitboard clear_3=18446744073692839935ULL;
//const bitboard clear_2=18446744073709486335ULL;
const bitboard clear_1=18446744073709551360ULL;

//const bitboard mask_8=18374686479671623680ULL;
const bitboard mask_7=71776119061217280ULL;
//const bitboard mask_6=280375465082880ULL;
//const bitboard mask_5=1095216660480ULL;
//const bitboard mask_4=4278190080ULL;
//const bitboard mask_3=16711680ULL;
const bitboard mask_2=65280ULL;
//const bitboard mask_1=255ULL;

inline void prtmv(const int &mv){
    char files[]={'a','b','c','d','e','f','g','h'};
    if(mv>4096){
        int f=(mv%4096)/64;
        int t=(mv%4096)%64;
        cout<<files[7-f%8]<<f/8+1<<files[7-t%8]<<t/8+1;
        switch (mv/4096){
            case 1:
                cout<<"=n";
                break;
            case 2:
                cout<<"=b";
                break;
            case 3:
                cout<<"=r";
                break;
            case 4:
                cout<<"=q";
                break;
        }
    }
    else{
        int f=mv/64;
        int t=mv%64;
        cout<<files[7-f%8]<<f/8+1<<files[7-t%8]<<t/8+1;
    }
    cout<<endl;
}

void generate_rook_tables(){
    for(int i=0;i<64;++i){
        bitboard piece=(1ULL << i);
        bitboard side=0;
        bitboard loc=piece;
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
            bitboard blockers=_pdep_u64(j,side);
            bitboard moves=0;
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
        bitboard piece=(1ULL << i);
        bitboard side=0;
        bitboard loc=piece;
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
            bitboard blockers=_pdep_u64(j,side);
            bitboard moves=0;
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

struct chess{
    bitboard wp=0ULL;
    bitboard wr=0ULL;
    bitboard wn=0ULL;
    bitboard wb=0ULL;
    bitboard wq=0ULL;
    bitboard wk=0ULL;

    bitboard bp=0ULL;
    bitboard br=0ULL;
    bitboard bn=0ULL;
    bitboard bb=0ULL;
    bitboard bq=0ULL;
    bitboard bk=0ULL;

    bitboard bpcs=0;
    bitboard wpcs=0;
    bitboard pieces=0;

    bool turn=true;
};

inline bitboard white_king_moves(const bitboard &piece, bitboard &wpcs){
    return (((piece & clear_h) << 7) | (piece << 8) | ((piece & clear_a) << 9) |
            ((piece & clear_a) << 1) | ((piece & clear_a) >> 7) | (piece >> 8) |
            ((piece & clear_h) >> 9) | ((piece & clear_h) >> 1)) &~wpcs;
}

inline bitboard black_king_moves(const bitboard &piece, bitboard &bpcs){
    return (((piece & clear_h) << 7) | (piece << 8) | ((piece & clear_a) << 9) |
            ((piece & clear_a) << 1) | ((piece & clear_a) >> 7) | (piece >> 8) |
            ((piece & clear_h) >> 9) | ((piece & clear_h) >> 1)) &~bpcs;
}

inline bitboard white_knight_moves(const bitboard &piece, bitboard &wpcs){
    return ((piece & clear_a & clear_b) >> 6 | (piece & clear_a) >> 15 | 
            (piece & clear_h) >> 17 | (piece & clear_h & clear_g) >> 10 |
            (piece & clear_h & clear_g) << 6 | (piece & clear_h) << 15 |
            (piece & clear_a) << 17 | (piece & clear_a & clear_b) << 10)
            &~wpcs;
}

inline bitboard black_knight_moves(const bitboard &piece, bitboard &bpcs){
    return ((piece & clear_a & clear_b) >> 6 | (piece & clear_a) >> 15 | 
            (piece & clear_h) >> 17 | (piece & clear_h & clear_g) >> 10 |
            (piece & clear_h & clear_g) << 6 | (piece & clear_h) << 15 |
            (piece & clear_a) << 17 | (piece & clear_a & clear_b) << 10)
            &~bpcs;
}

inline bitboard white_pawn_moves(const bitboard &piece, chess &b){
    return (((piece << 8) | ((((piece&mask_2)<<8)&~b.pieces)<<8)) &~ b.pieces) | 
            ((((piece & clear_h) << 7) | ((piece & clear_a) << 9)) & b.bpcs);
}

inline bitboard black_pawn_moves(const bitboard &piece, chess &b){
    return (((piece >> 8) | ((((piece&mask_7)>>8)&~b.pieces)>>8))&~b.pieces) | 
            ((((piece & clear_a) >> 7) | ((piece & clear_h) >> 9)) & b.wpcs);
}

inline bitboard white_bishop_moves(const int &bpos, chess &b){
    return (bishop_moves_lkup[bpos][_pext_u64(b.pieces,bishop_blockers[bpos])])&~b.wpcs;
}

inline bitboard black_bishop_moves(const int &bpos, chess &b){
    return (bishop_moves_lkup[bpos][_pext_u64(b.pieces,bishop_blockers[bpos])])&~b.bpcs;
}

inline bitboard white_rook_moves(const int &rpos, chess &b){
    return (rook_moves_lkup[rpos][_pext_u64(b.pieces,rook_blockers[rpos])])&~b.wpcs;
}

inline bitboard black_rook_moves(const int &rpos, chess &b){
    return (rook_moves_lkup[rpos][_pext_u64(b.pieces,rook_blockers[rpos])])&~b.bpcs;
}

inline bitboard white_queen_moves(const int &qpos, chess &b){
    return (bishop_moves_lkup[qpos][_pext_u64(b.pieces,bishop_blockers[qpos])] |
            rook_moves_lkup[qpos][_pext_u64(b.pieces,rook_blockers[qpos])])&~b.wpcs;
}

inline bitboard black_queen_moves(const int &qpos, chess &b){
    return (bishop_moves_lkup[qpos][_pext_u64(b.pieces,bishop_blockers[qpos])] |
            rook_moves_lkup[qpos][_pext_u64(b.pieces,rook_blockers[qpos])])&~b.bpcs;
}

inline vector<int> moves_white(chess &b){
    vector<int> moves;
    bitboard w;
    int j;

    bitboard v=b.wb;
    int i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=white_bishop_moves(i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.wk;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=white_king_moves(1ULL << i,b.wpcs);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.wn;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=white_knight_moves(1ULL << i,b.wpcs);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.wp;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=white_pawn_moves(1ULL << i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.wq;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=white_queen_moves(i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.wr;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=white_rook_moves(i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.wp&mask_7;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i+8))&b.pieces)==0){
            moves.push_back(i*65+4104);
            moves.push_back(i*65+8200);
            moves.push_back(i*65+12296);
            moves.push_back(i*65+16392);
        }
        v>>=1;
    }
    v=b.wp&mask_7&clear_h;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i+7))&b.bpcs)!=0){
            moves.push_back(i*65+4103);
            moves.push_back(i*65+8199);
            moves.push_back(i*65+12295);
            moves.push_back(i*65+16391);
        }
        v>>=1;
    }
    v=b.wp&mask_7&clear_a;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i+9))&b.bpcs)!=0){
            moves.push_back(i*65+4105);
            moves.push_back(i*65+8201);
            moves.push_back(i*65+12297);
            moves.push_back(i*65+16393);
        }
        v>>=1;
    }
    return moves;
}

inline vector<int> moves_black(chess &b){
    vector<int> moves;
    bitboard w;
    int j;

    bitboard v=b.bb;
    int i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=black_bishop_moves(i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.bk;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=black_king_moves(1ULL << i,b.bpcs);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.bn;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=black_knight_moves(1ULL << i,b.bpcs);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.bp;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=black_pawn_moves(1ULL << i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.bq;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=black_queen_moves(i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.br;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        j=-1;
        w=black_rook_moves(i,b);
        while(w!=0){
            j+=__builtin_ctzll(w)+1;
            w>>=__builtin_ctzll(w);
            moves.push_back(i*64+j);
            w>>=1;
        }
        v>>=1;
    }
    v=b.bp&mask_2;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i-8))&b.pieces)==0){
            moves.push_back(i*65+4088);
            moves.push_back(i*65+8184);
            moves.push_back(i*65+12280);
            moves.push_back(i*65+16376);
        }
        v>>=1;
    }
    v=b.bp&mask_2&clear_a;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i-7))&b.wpcs)!=0){
            moves.push_back(i*65+4089);
            moves.push_back(i*65+8185);
            moves.push_back(i*65+12281);
            moves.push_back(i*65+16377);
        }
        v>>=1;
    }
    v=b.bp&mask_2&clear_h;
    i=-1;
    while(v!=0){
        i+=__builtin_ctzll(v)+1;
        v>>=__builtin_ctzll(v);
        if(((1ULL<<(i-9))&b.wpcs)!=0){
            moves.push_back(i*65+4087);
            moves.push_back(i*65+8183);
            moves.push_back(i*65+12279);
            moves.push_back(i*65+16375);
        }
        v>>=1;
    }
    return moves;
}

inline vector<int> pl_moves(chess &b){
    switch (b.turn){
        case true:
            return moves_white(b);
        case false:
            return moves_black(b);
    }
}

inline void move_piece_white(const int &mv, chess &b){
    if(mv>4096){
        bitboard t=~(1ULL<<((mv%4096)%64));
        b.wp &= ~(1ULL << ((mv%4096)/64));
        b.bb &= t;
        b.bk &= t;
        b.bn &= t;
        b.bp &= t;
        b.bq &= t;
        b.br &= t;
        switch(mv/4096){
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
        bitboard f=(1ULL<<(mv/64));
        bitboard t=~(1ULL<<(mv%64));
        b.bb &= t;
        b.bk &= t;
        b.bn &= t;
        b.bp &= t;
        b.bq &= t;
        b.br &= t;
        if((b.wb&f)!=0)
            b.wb=(b.wb&(~f))|~t;
        else if((b.wk&f)!=0)
            b.wk=(b.wk&(~f))|~t;
        else if((b.wn&f)!=0)
            b.wn=(b.wn&(~f))|~t;
        else if((b.wp&f)!=0)
            b.wp=(b.wp&(~f))|~t;
        else if((b.wq&f)!=0)
            b.wq=(b.wq&(~f))|~t;
        else
            b.wr=(b.wr&(~f))|~t;
    }
    b.turn=false;
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline void move_piece_black(const int &mv, chess &b){
   if(mv>4096){
        bitboard t=~(1ULL << ((mv%4096)%64));
        b.bp &= ~(1ULL << ((mv%4096)/64));
        b.wb &= t;
        b.wk &= t;
        b.wn &= t;
        b.wp &= t;
        b.wq &= t;
        b.wr &= t;
        switch(mv/4096){
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
        bitboard f=(1ULL<<(mv/64));
        bitboard t=~(1ULL<<(mv%64));
        b.wb &= t;
        b.wk &= t;
        b.wn &= t;
        b.wp &= t;
        b.wq &= t;
        b.wr &= t;
        if((b.bb&f)!=0)
            b.bb=(b.bb&(~f))|~t;
        else if((b.bk&f)!=0)
            b.bk=(b.bk&(~f))|~t;
        else if((b.bn&f)!=0)
            b.bn=(b.bn&(~f))|~t;
        else if((b.bp&f)!=0)
            b.bp=(b.bp&(~f))|~t;
        else if((b.bq&f)!=0)
            b.bq=(b.bq&(~f))|~t;
        else
            b.br=(b.br&(~f))|~t;
    }
    b.turn=true;
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline void move_piece(const int &mv, chess &b){
    switch(b.turn){
        case true:
            return move_piece_white(mv,b);
        case false:
            return move_piece_black(mv,b);
    }
}

inline void parse_fen(string &fen, chess &b){
    b.wp=0ULL; b.wr=0ULL; b.wn=0ULL; b.wb=0ULL; b.wq=0ULL; b.wk=0ULL;
    b.bp=0ULL; b.br=0ULL; b.bn=0ULL; b.bb=0ULL; b.bq=0ULL; b.bk=0ULL;
    for (char c : fen) {
        if (isdigit(c)) {
            b.wp<<=c-'0';
            b.wr<<=c-'0';
            b.wn<<=c-'0';
            b.wb<<=c-'0';
            b.wq<<=c-'0';
            b.wk<<=c-'0';
            b.bp<<=c-'0';
            b.br<<=c-'0';
            b.bn<<=c-'0';
            b.bb<<=c-'0';
            b.bq<<=c-'0';
            b.bk<<=c-'0';
        }
        else if (c!='/'){
            b.wp<<=1;
            b.wr<<=1;
            b.wn<<=1;
            b.wb<<=1;
            b.wq<<=1;
            b.wk<<=1;
            b.bp<<=1;
            b.br<<=1;
            b.bn<<=1;
            b.bb<<=1;
            b.bq<<=1;
            b.bk<<=1;
            switch(c){
                case 'p':
                    b.bp+=1;
                    break;
                case 'n':
                    b.bn+=1;
                    break;
                case 'b':
                    b.bb+=1;
                    break;
                case 'r':
                    b.br+=1;
                    break;
                case 'q':
                    b.bq+=1;
                    break;
                case 'P':
                    b.wp+=1;
                    break;
                case 'N':
                    b.wn+=1;
                    break;
                case 'B':
                    b.wb+=1;
                    break;
                case 'R':
                    b.wr+=1;
                    break;
                case 'Q':
                    b.wq+=1;
                    break;
                case 'k':
                    b.bk+=1;
                    break;
                case 'K':
                    b.wk+=1;
                    break;
            }
        }
    }
    b.bpcs=b.bp|b.br|b.bn|b.bb|b.bq|b.bk;
    b.wpcs=b.wp|b.wr|b.wn|b.wb|b.wq|b.wk;
    b.pieces=b.bpcs|b.wpcs;
}

inline int seval(chess &b){
    ++nodes;
    if (b.turn)
        return ((__builtin_popcountll(b.wb)*4) +
                (__builtin_popcountll(b.wn)*3) +
                (__builtin_popcountll(b.wp)) +
                (__builtin_popcountll(b.wq)*9) +
                (__builtin_popcountll(b.wk)*64) +
                (__builtin_popcountll(b.wr)*5) -
                (__builtin_popcountll(b.bb)*4) -
                (__builtin_popcountll(b.bn)*3) -
                (__builtin_popcountll(b.bp)) -
                (__builtin_popcountll(b.bq)*9) -
                (__builtin_popcountll(b.br)*5)-
                (__builtin_popcountll(b.bk)*64));
    return ((__builtin_popcountll(b.bb)*4) +
                (__builtin_popcountll(b.bn)*3) +
                (__builtin_popcountll(b.bp)) +
                (__builtin_popcountll(b.bq)*9) +
                (__builtin_popcountll(b.bk)*64) +
                (__builtin_popcountll(b.br)*5) -
                (__builtin_popcountll(b.wb)*4) -
                (__builtin_popcountll(b.wn)*3) -
                (__builtin_popcountll(b.wp)) -
                (__builtin_popcountll(b.wq)*9) -
                (__builtin_popcountll(b.wr)*5)-
                (__builtin_popcountll(b.wk)*64));
}
int minimax(chess &b, int depth, int alpha, int beta){
    if (depth <= 1)
        return seval(b);
    //check for checkmates, higher eval for delayed checkmate
    if(b.wk==0){
        if (b.turn)
            return -128-depth;
        return 128+depth;
    }
    if (b.bk==0){
        if (b.turn)
            return 128+depth;
        return -128-depth;
    }
    //declare vars
    chess rd;
    //get pl moves
    vector<int> mvs=pl_moves(b);
    //move ordering
    int advs[mvs.size()];
    for (int i=0;i<(int)mvs.size();++i){
        rd=b;
        move_piece(mvs[i],rd);
        advs[i]=-seval(rd);
    }
    //evaluate advantage
    for(int i=0;i<(int)mvs.size();++i){
        //get the maximum advantage for highest ab efficiency
        int maxi=advs[0];
        int maxelem=0;
        for(int j=1;j<(int)mvs.size();++j){
            if(advs[j]>maxi){
                maxi=advs[j];
                maxelem=j;
            }
        }
        advs[maxelem]=-9999;
        rd=b;
        move_piece(mvs[maxelem],rd);
        alpha=max(-minimax(rd,depth-1,-beta,-alpha),alpha);
        if(alpha>=beta){return alpha;}
    }
    return alpha;
}


int playbest(chess &b){
    chess rd;
    int eval;
    int alpha=-999;
    int beta=999;
    int bestmove=0;
    vector<int> mvs=pl_moves(b);
    int advs[mvs.size()];
    for (int i=0;i<(int)mvs.size();++i){
        rd=b;
        move_piece(mvs[i],rd);
        advs[i]=-seval(rd);
    }
    //evaluate advantage
    for(int i=0;i<(int)mvs.size();++i){
        //get the maximum advantage for highest ab efficiency
        int maxi=advs[0];
        int maxelem=0;
        for(int j=1;j<(int)mvs.size();++j){
            if(advs[j]>maxi){
                maxi=advs[j];
                maxelem=j;
            }
        }
        advs[maxelem]=-9999;
        rd=b;
        move_piece(mvs[maxelem],rd);
        eval=-minimax(rd,mdepth,-beta,-alpha);
        if(eval>alpha){
            alpha=eval;
            bestmove=mvs[maxelem];
        }
        if(alpha>=beta){break;}
    }
    return bestmove;
}

int id_minimax(chess &b, int depth, int alpha, int beta, timept &start, int &time){
    if (depth <= 1)
        return seval(b);
    //check for checkmates, higher eval for delayed checkmate
    if(b.wk==0){
        if (b.turn)
            return -128-depth;
        return 128+depth;
    }
    if (b.bk==0){
        if (b.turn)
            return 128+depth;
        return -128-depth;
    }
    //declare vars
    chess rd;
    //get pl moves
    vector<int> mvs=pl_moves(b);
    //move ordering
    int advs[mvs.size()];
    for (int i=0;i<(int)mvs.size();++i){
        rd=b;
        move_piece(mvs[i],rd);
        advs[i]=-seval(rd);
    }
    //evaluate advantage
    for(int i=0;i<(int)mvs.size();++i){
        //get the maximum advantage for highest ab efficiency
        int maxi=advs[0];
        int maxelem=0;
        for(int j=1;j<(int)mvs.size();++j){
            if(advs[j]>maxi){
                maxi=advs[j];
                maxelem=j;
            }
        }
        advs[maxelem]=-9999;
        rd=b;
        move_piece(mvs[maxelem],rd);
        alpha=max(-minimax(rd,depth-1,-beta,-alpha),alpha);
        if(alpha>=beta){return alpha;}
        if((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-start).count())>time){
            break;
        }
    }
    return alpha;
}

int iterative_deepening(chess &b, int &t){
    auto s=chrono::high_resolution_clock::now();
    int d=2;
    int bm=0;
    int besteval=-999;
    while((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count())<t){
        chess rd;
        int eval;
        int bestmove=0;
        int alpha=-999;
        int beta=999;
        vector<int> mvs=pl_moves(b);
        int advs[mvs.size()];
        for (int i=0;i<(int)mvs.size();++i){
            rd=b;
            move_piece(mvs[i],rd);
            advs[i]=-seval(rd);
        }
        //evaluate advantage
        for(int i=0;i<(int)mvs.size();++i){
            //get the maximum advantage for highest ab efficiency
            int maxi=advs[0];
            int maxelem=0;
            for(int j=1;j<(int)mvs.size();++j){
                if(advs[j]>maxi){
                    maxi=advs[j];
                    maxelem=j;
                }
            }
            advs[maxelem]=-9999;
            rd=b;
            move_piece(mvs[maxelem],rd);
            eval=-id_minimax(rd,d,-beta,-alpha,s,t);
            if((int)(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-s).count())>t){
                if(alpha>besteval)
                    return bestmove; 
                return bm;
            }
            if(eval>alpha){
                alpha=eval;
                bestmove=mvs[maxelem];
            }
            if(alpha>=beta){break;}
        }
        besteval=alpha;
        bm=bestmove;
        d+=1;
    }
    return bm;
}

void profile(int iters=5, string fen="r1bqkb1r/p1pp1ppp/1p6/2n1p3/1QBn4/N1P5/PP1P1PPP/R1B1K1NR"){
    chess b;
    parse_fen(fen,b);
    auto start=chrono::high_resolution_clock::now();
    for(int i=1;i<=iters;++i){
        int best_move=playbest(b);
        move_piece(best_move,b);
        cout<<i<<"-";
        prtmv(best_move);
    }
    cout<<endl;
    auto stop=chrono::high_resolution_clock::now();
    cout<<"time per move:"<<chrono::duration_cast<chrono::microseconds>(stop - start).count()/((double)1000000*iters)<<endl;
    cout<<"nps:"<<nodes*(double)1000000/chrono::duration_cast<chrono::microseconds>(stop - start).count()<<endl;
    cout<<"nodes per move:"<<nodes/(double)iters;
}

int main(){
    generate_rook_tables();
    generate_bishop_tables();

<<<<<<< HEAD
    profile(10);

    /*int tim=10000;
=======

    int tim=700;
>>>>>>> 5c4f716 (First)
    chess brd;
    string fen;
    char t;
    while(true){
        cin>>fen;
        cin>>t;
        if (t=='w')
            brd.turn=true;
        else
            brd.turn=false;
        
        parse_fen(fen,brd);
        prtmv(iterative_deepening(brd,tim));
<<<<<<< HEAD
    }*/
=======
    }
>>>>>>> 5c4f716 (First)
    return 0;
}