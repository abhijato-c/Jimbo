#include <iostream>
#include <vector>
#include <chrono>
#include <immintrin.h>

using namespace std;
typedef unsigned long long int bitboard;

int main(){
    bitboard a = 9223372036854775808ULL;
    a>>=__builtin_ctzll(a)+1;
    cout<<a<<endl;
}