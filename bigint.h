#ifndef BIGINT_BIGINT_H_
#define BIGINT_BIGINT_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stddef.h>

typedef unsigned long long bigint_chunk;
const int kBigintByteBits = 8; // number of bits per byte
const int kBigintChunkBits = sizeof(bigint_chunk) * kBigintByteBits - 1;

typedef struct Bigint {
  bigint_chunk* chunks;
  size_t capacity;
  size_t size;
  size_t bits;
  int sign;
} Bigint;

Bigint* bigint_new(size_t capacity);
Bigint* bigint_copy(Bigint* num);
void bigint_reserve(Bigint* num, size_t new_capacity);
void bigint_destroy(Bigint* num);
Bigint* atobi(const char* s_in);
char* bitoa(const Bigint* num);
void bigint_add_assign(Bigint* a, const Bigint* b);
Bigint* bigint_mul_bigint(const Bigint* a, const Bigint* b);


#ifdef __cplusplus
}
#endif //__cplusplus
#endif //BIGINT_BIGINT_H_