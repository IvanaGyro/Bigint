#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "bigint.h"

Bigint bigint_default = {NULL, 0, 0, 0, 1};

inline void* bigint_malloc(size_t size) {
    void* memory = malloc(size);
    if (!memory) {
      perror("bigint_malloc");
      exit(errno);
    }
    return memory;
}

inline void* bigint_calloc(size_t number, size_t size) {
    void* memory = calloc(number, size);
    if (!memory) {
      perror("bigint_calloc");
      exit(errno);
    }
    return memory;
}

inline void* bigint_realloc(void* memory, size_t new_size) {
    memory = realloc(memory, new_size);
    if (!memory) {
      perror("bigint_realloc");
      exit(errno);
    }
    return memory;
}

Bigint* bigint_copy(Bigint* num) {
  Bigint* copy = (Bigint*)bigint_malloc(sizeof(Bigint));
  *copy = *num;
  copy->chunks = (bigint_chunk*)bigint_malloc(copy->capacity * kBigintChunkSize);
  memcpy(copy->chunks, num->chunks, copy->capacity * kBigintChunkSize);
  return copy;
}

void bigint_destroy(Bigint* num) {
  free(num->chunks);
  free(num);
}

inline void update_size(Bigint* n, size_t chunk_bits) {
  // update size according to bits
  n->size = (n->bits + chunk_bits - 1) / chunk_bits;
}

inline void trim_bits_by_one(Bigint* n, size_t chunk_bits) {
  // The input bigint's bits should be correct or bigger than the coorect value
  // just by one.
  if (n->bits == 0) return;
  size_t tmp = n->bits + chunk_bits - 1;
  size_t pos = tmp / chunk_bits - 1;
  size_t offset = tmp % chunk_bits;
  bigint_chunk mask = 1;
  if ((n->chunks[pos] & mask << offset) == 0) --n->bits;
}

Bigint* atobi(const char* s_in) {
  int len = strlen(s_in);
  char* buf = (char*)bigint_malloc(len + 1);
  strcpy(buf, s_in);
  int sign = 1;
  char*s = buf;
  while (*s == '+' || *s == '-') {
    sign = *s == '+' ? sign : -sign;
    ++s;
    --len;
  }
  while (*s == '0') {
    ++s;
    --len;
  }
  Bigint* num = (Bigint*)bigint_malloc(sizeof(Bigint));
  *num = bigint_default;
  if (len == 0) {
    num->chunks = (bigint_chunk*)bigint_malloc(kBigintChunkSize);
    num->chunks[0] = 0;
    num->capacity = 1;
    num->size = 0;
    num->bits = 0;
    return num;
  }
  
  num->capacity = (int)(len * kBigintLog2_10 / kBigintChunkBits + 1);
  num->chunks = (bigint_chunk*)bigint_calloc(num->capacity, kBigintChunkSize);
  num->sign = sign;

  int tail = len - 1;
  bigint_chunk* cur_chunk = num->chunks;
  bigint_chunk cur_mask = 1;
  char carry, tmp;
  int i, l, r;

  while (*s) {
    ++num->bits;
    if (s[tail] & 1) { // '1' = 49, '3' = 51...
      *cur_chunk |= cur_mask;
    }
    // divide s by 2
    for (r = 0, l = 0, carry = 0; r <= tail; ++r) {
      tmp = carry * 10 + (s[r] - '0');
      carry = tmp & 1;
      if (l != 0 || tmp >= 2) {
        s[l++] = '0' + tmp / 2;
      }
    }
    s[l] = '\0';
    tail = l - 1;
    // shift mask
    cur_mask <<= 1;
    if (cur_mask == kBigintMask) {
      cur_mask = 1;
      ++cur_chunk;
    }
  }
  free(buf);
  update_size(num, kBigintChunkBits);
  return num;
}

char* bitoa(const Bigint* num) {
  // In mul(), we assume that the multiplication causes a carry, so here,
  // we need one btye of extra memory.
  // For example, "9" * "1" = "9", the len of the result is 1. However, in 
  // mul(), due to the assumation that the maximum length of the result of
  // multiplication of 1 digit by 1 digit is 2 digits, mul() needs 2 bytes
  // to work correctly.
  int off = num->sign == 1 ? 0 : 1;
  char* res = (char*)bigint_calloc(num->bits * kBigintLog2 + 2 + 1 + off, 1);
  char* s = res;
  if (num->sign == -1) *s++ = '-'; 
  *s = '0';
  char* buf = (char*)bigint_malloc(kBigintChunkBits * kBigintLog2 + 2);
  
  int i;
  char* l;
  char* r;
  char tmp;
  bigint_chunk mask, upper_bound = kBigintMask >> 1;
  bigint_chunk multiplier = 1;
  for (i=num->size-1; i >= 0; --i) {
    mask = upper_bound;
    while (mask) {
      multiplier <<= 1;
      if (num->chunks[i] & mask || multiplier == upper_bound) {
        _i64toa(multiplier, buf, 10);
        r = buf + strlen(buf) - 1;
        l = buf;
        while (r > l) {
          tmp = *r;
          *r-- = *l;
          *l++ = tmp;
        }
        multiplier = 1;
        mul(s, buf);
        if (num->chunks[i] & mask) add(s, "1");
      }
      mask >>= 1;
    }
  }
  _i64toa(multiplier, buf, 10);
  r = buf + strlen(buf) - 1;
  l = buf;
  while (r > l) {
    tmp = *r;
    *r-- = *l;
    *l++ = tmp;
  }
  mul(s, buf);
  free(buf);
  r = s + strlen(s) - 1;
  l = s;
  while (r > l) {
    tmp = *r;
    *r-- = *l;
    *l++ = tmp;
  }
  return res;
}

void mul(char* a, const char* b) {
  int al = strlen(a), bl = strlen(b);
  char* p = a + al - 1;
  while (*p == '0' && al) {
    *p-- = '\0';
    --al;
  }
  if (al == 0) {
    *++p = '0';
    return;
  }

  char* buf = (char*)bigint_malloc(al + bl + 1);
  char* a_copy = (char*)bigint_malloc(al + 1);
  strcpy(a_copy, a);
  int i, j, k;
  char carry = 0, na, nb;
  for (i = 0; i < al + bl + 1; ++i) a[i] = '\0';
  for (i = 0; i < bl; ++i) {
    for (k = 0; k < i; ++k) {
      buf[k] = '0';
    }
    for (j = 0; j < al; ++j, ++k) {
      na = a_copy[j] - '0';
      nb = b[i] - '0';
      na = na * nb + carry;
      carry = na / 10;
      buf[k] = na % 10 + '0';
    }
    if (carry) {
      buf[k++] = carry + '0';
      carry = 0;
    }
    buf[k] = '\0';
    add(a, buf);
  }
  free(buf);
  free(a_copy);
}

void add(char* a, const char* b) {
  char carry = 0, na, nb;
  char* pa = a;
  const char* pb = b;
  while (*pb) {
    na = *pa >= '0' ? *pa-'0' : 0;
    nb = *pb++ - '0';
    na += nb + carry;
    carry = na / 10;
    *pa++ = na % 10 + '0';
  }
  while (carry) {
    na = *pa >= '0' ? *pa-'0' : 0;
    ++na;
    carry = na / 10;
    *pa++ = na % 10 + '0';
  }
}

void bigint_add_assign(Bigint* a, const Bigint* b) {
  int bits = (a->bits > b->bits ? a->bits : b->bits) + 1;
  int len = (bits + kBigintChunkBits - 1) / kBigintChunkBits;
  if (a->capacity < len) {
    a->chunks = (bigint_chunk*)bigint_realloc(a->chunks, len * kBigintChunkSize);
    memset(a->chunks + a->capacity, 0, (len - a->capacity) * kBigintChunkSize);
    a->capacity = len;
  }

  bigint_chunk carry = 0;
  int i;
  for (i = 0; i < b->size; ++i) {
    a->chunks[i] += b->chunks[i] + carry;
    carry = (kBigintMask & a->chunks[i]) == kBigintMask;
    a->chunks[i] &= kBigintMask - 1;
  }
  while (carry) {
    a->chunks[i] += carry;
    carry = (kBigintMask & a->chunks[i]) == kBigintMask;
    a->chunks[i] &= kBigintMask - 1;
    ++i;
  }
  a->bits = bits;
  trim_bits_by_one(a, kBigintChunkBits);
  update_size(a, kBigintChunkBits);
  // if (i) { // a and b are not both 0
  //   int pos = (bits - 1) / kBigintChunkBits;
  //   int off = (bits - 1) % kBigintChunkBits;
  //   bigint_chunk mask = 1;
  //   if (a->chunks[i-1] & mask << off) a->bits = bits;
  //   else a->bits = bits-1;
    
  // }
}

// The comments show the value of each constant when `bigint_chunk` is 8-bits
// long.
const int kHighMulCarryOffset = kBigintChunkBits>> 1; // 3
const int kLowMulCarryOffset = kHighMulCarryOffset + 1; // 4
const int kHalfOffset = kLowMulCarryOffset; // 4
const bigint_chunk kHalfBitMask = (bigint_chunk)1 << kLowMulCarryOffset; // 0b 0001 0000
const bigint_chunk kLowHalfMask = kHalfBitMask - 1; // 0b 0000 1111
const bigint_chunk kHighHalfMask = ~kLowHalfMask; // 0b 1111 0000
const bigint_chunk kHighMulMask = ~(kLowHalfMask >> 1); // 0b 1111 1000
const bigint_chunk kLowMulMask = kHighHalfMask; // 0b 1111 0000

Bigint* split(const Bigint* n) {
  Bigint* res = (Bigint*)bigint_malloc(sizeof(Bigint));
  *res = *n;
  res->capacity = n->size == 0 ? 1 : n->size << 1;
  res->size = res->capacity;
  res->chunks = (bigint_chunk*)bigint_calloc(res->capacity, kBigintChunkSize);

  bigint_chunk* p = res->chunks;
  bigint_chunk cur = n->chunks[0];
  int need = kHalfOffset,
      remainder = kBigintChunkBits,
      i = 0;
  while (i < n->size) {
    if (remainder >= need) {
      *p++ |= (cur & ((bigint_chunk)1 << need) - 1) << kHalfOffset - need;
      if (remainder == need) {
        cur = n->chunks[++i];
        remainder = kBigintChunkBits;
      } else {
        cur >>= need;
        remainder -= need;
      }
      need = kHalfOffset;
    } else {
      *p |= cur << kHalfOffset - need;
      need -= remainder;
      cur = n->chunks[++i];
      remainder = kBigintChunkBits;
    }
  }
  if (res->chunks[res->size - 1] == 0 && res->size > 0) --res->size;
  return res;
}

void combine(Bigint* n) {
  int need = kBigintChunkBits,
      remainder = kHalfOffset;
  bigint_chunk* l = n->chunks;
  bigint_chunk* r = n->chunks;
  bigint_chunk* end = n->chunks + n->size;
  bigint_chunk cur = *r;
  *r = 0;
  while (r < end) {
    if (remainder >= need) {
      *l++ |= (cur & ((bigint_chunk)1 << need) - 1) << kBigintChunkBits - need;
      if (remainder == need) {
        cur = *++r;
        if (r < end) *r = 0;
        remainder = kHalfOffset;
      } else {
        cur >>= need;
        remainder -= need;
      }
      need = kBigintChunkBits;
    } else {
      *l |= cur << kBigintChunkBits - need;
      need -= remainder;
      cur = *++r;
      if (r < end) *r = 0;
      remainder = kHalfOffset;
    }
  }
  update_size(n, kBigintChunkBits);
}

Bigint* bigint_mul_bigint_school(const Bigint* a, const Bigint* b) {
  Bigint* a_split = split(a);
  Bigint* b_split = split(b);
  int a_len = a_split->size, b_len = b_split->size;
  int len = a_len + b_len;
  Bigint* res = (Bigint*)bigint_malloc(sizeof(Bigint));
  res->capacity = len;
  res->sign = a->sign * b->sign;
  res->chunks = (bigint_chunk*)bigint_calloc(len, kBigintChunkSize);
  bigint_chunk* buf = (bigint_chunk*)bigint_malloc(len * kBigintChunkSize);
  bigint_chunk tmp, carry_mul = 0, carry_add = 0;
  for (int i = 0; i < b_len; ++i) {
    int j = 0;
    for (j = 0; j < a_len; ++j) {
      tmp = a_split->chunks[j] * b_split->chunks[i] + carry_mul;
      carry_mul = (tmp & kHighHalfMask) >> kHalfOffset;
      res->chunks[i + j] += (tmp & kLowHalfMask) + carry_add;
      carry_add = (res->chunks[i + j] & kHighHalfMask) != 0;
      res->chunks[i + j] &= kLowHalfMask;
    }
    carry_add += carry_mul;
    while (carry_add) {
      res->chunks[i + j] += carry_add;
      carry_add = ((res->chunks[i + j] & kHighHalfMask) >> kHalfOffset) & kLowHalfMask;
      res->chunks[i + j++] &= kLowHalfMask;
    }
    carry_add = carry_mul = 0;
  }
  if (res->chunks[res->capacity-1] == 0) res->size = res->capacity-1;
  else res->size = res->capacity;
  res->bits = a->bits + b->bits;
  trim_bits_by_one(res, kHalfOffset);
  combine(res);
  return res;
}

#ifdef __cplusplus
}
#endif //__cplusplus
