#ifndef BIGINT_BIGINT_H_
#define BIGINT_BIGINT_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stddef.h>

typedef unsigned long long bigint_chunk;
const int kBigintChunkSize = sizeof(bigint_chunk);
const int kBigintByteBits = 8; // number of bits per byte
const int kBigintChunkBits = kBigintChunkSize * kBigintByteBits - 1;
const bigint_chunk kBigintMask = (bigint_chunk)1 << kBigintChunkBits;
const double kBigintLog2_10 = 3.3219280948873621817;
const double kBigintLog2 = 0.30102999566398119802;

extern struct Bigint {
  bigint_chunk* chunks;
  size_t capacity;
  size_t size;
  size_t bits;
  int sign;
} bigint_default;
typedef struct Bigint Bigint;

Bigint* bigint_copy(Bigint* num);
void bigint_destroy(Bigint* num);
Bigint* atobi(const char* s_in);
char* bitoa(const Bigint* num);
void add(char* a, const char* b);
void mul(char* a, const char* b);
void bigint_add_assign(Bigint* a, const Bigint* b);
Bigint* bigint_mul_bigint(const Bigint* a, const Bigint* b);


#ifdef __cplusplus
}
#endif //__cplusplus
#endif //BIGINT_BIGINT_H_