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

inline void prtmv(const Move &mv){
    char files[]={'a','b','c','d','e','f','g','h'};
    int from = (mv & 63);
    int to = ((mv>>6) & 63);
    int prom = ((mv>>12) & 7);
    cout << files[7 - from%8] << (from/8)+1 << files[7 - to%8] << (to/8)+1;
    if (prom>0){
        switch(prom){
            case 1:
                cout << "=n";
                break;
            case 2:
                cout << "=b";
                break;
            case 3:
                cout << "=r";
                break;
            case 4:
                cout << "=q";
                break;
        }
    }
    cout<<endl;
}

inline void ParseFEN(string fen, chess &b){
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