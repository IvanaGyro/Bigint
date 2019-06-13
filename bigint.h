#ifndef BIGINT_BIGINT_H_
#define BIGINT_BIGINT_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "math.h"

typedef unsigned long long bigint_chunk;
const int kBigintChunkSize = sizeof(bigint_chunk);
const int kBigintByteBits = 8; // number of bits per byte
const int kBigintChunkBits = kBigintChunkSize * kBigintByteBits - 1;
const bigint_chunk kBigintMask = (bigint_chunk)1 << kBigintChunkBits;
const double kBigintLog2_10 = 3.3219280948873621817;
const double kBigintLog2 = 0.30102999566398119802;

extern struct Bigint {
  bigint_chunk* chunks;
  int len;
  int bits;
} bigint_default;
typedef struct Bigint Bigint;

int atobi(const char* in, bigint_chunk** out);
char* bitoa(const bigint_chunk* num, int len);
void add(char* a, const char* b);
void mul(char* a, const char* b);


#ifdef __cplusplus
}
#endif //__cplusplus
#endif //BIGINT_BIGINT_H_