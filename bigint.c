#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "bigint.h"

typedef bigint_chunk chunk;
const double kLog2_10 = 3.3219280948873621817;
const double kLog2 = 0.30102999566398119802;

// The comments show the value of each constant when `chunk` is 8-bits
// long.
const size_t kHalfOffset = (kBigintChunkBits>> 1) + 1; // 4
const chunk kBitMask = (chunk)1 << kBigintChunkBits; // 0b 1000 0000
const chunk kHalfBitMask = (chunk)1 << kHalfOffset; // 0b 0001 0000
const chunk kLowHalfMask = kHalfBitMask - 1; // 0b 0000 1111
const chunk kHighHalfMask = ~kLowHalfMask; // 0b 1111 0000

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

Bigint* bigint_new(size_t capacity) {
  Bigint* num = (Bigint*)bigint_calloc(1, sizeof(Bigint));
  bigint_reserve(num, capacity);
  num->sign = 1;
  return num;
}

Bigint* bigint_copy(Bigint* num) {
  Bigint* copy = (Bigint*)bigint_malloc(sizeof(Bigint));
  *copy = *num;
  copy->chunks = (chunk*)bigint_malloc(copy->capacity * sizeof(chunk));
  memcpy(copy->chunks, num->chunks, copy->capacity * sizeof(chunk));
  return copy;
}

void bigint_reserve(Bigint* num, size_t new_capacity) {
  size_t capacity = num->capacity;
  if (capacity < new_capacity) {
    if (capacity == 0) {
      num->chunks = (chunk*)bigint_calloc(new_capacity, sizeof(chunk));
    } else {
      num->chunks = (chunk*)bigint_realloc(num->chunks, 
                                           new_capacity * sizeof(chunk));
      memset(num->chunks + capacity,
             0, (new_capacity - capacity) * sizeof(chunk));
    }
    num->capacity = new_capacity;
  }
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
  chunk mask = 1;
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

  if (len == 0) return bigint_new(0);
  Bigint* num = bigint_new(len * kLog2_10 / kBigintChunkBits + 1);
  num->sign = sign;

  int tail = len - 1;
  chunk* cur_chunk = num->chunks;
  chunk cur_mask = 1;
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
    if (cur_mask == kBitMask) {
      cur_mask = 1;
      ++cur_chunk;
    }
  }
  free(buf);
  update_size(num, kBigintChunkBits);
  return num;
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

char* bitoa(const Bigint* num) {
  // In mul(), we assume that the multiplication causes a carry, so here,
  // we need one btye of extra memory.
  // For example, "9" * "1" = "9", the len of the result is 1. However, in 
  // mul(), due to the assumation that the maximum length of the result of
  // multiplication of 1 digit by 1 digit is 2 digits, mul() needs 2 bytes
  // to work correctly.
  int off = num->sign == 1 ? 0 : 1;
  char* res = (char*)bigint_calloc(num->bits * kLog2 + 2 + 1 + off, 1);
  char* s = res;
  if (num->sign == -1) *s++ = '-'; 
  *s = '0';
  if (num->size == 0) return res;
  char* buf = (char*)bigint_malloc(kBigintChunkBits * kLog2 + 2);
  
  int i;
  char* l;
  char* r;
  char tmp;
  chunk mask, upper_bound = kBitMask >> 1;
  chunk multiplier = 1;
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

void bigint_add_assign(Bigint* a, const Bigint* b) {
  int bits = (a->bits > b->bits ? a->bits : b->bits) + 1;
  int len = (bits + kBigintChunkBits - 1) / kBigintChunkBits;
  bigint_reserve(a, len);

  chunk carry = 0;
  int i;
  for (i = 0; i < b->size; ++i) {
    a->chunks[i] += b->chunks[i] + carry;
    carry = (kBitMask & a->chunks[i]) == kBitMask;
    a->chunks[i] &= kBitMask - 1;
  }
  while (carry) {
    a->chunks[i] += carry;
    carry = (kBitMask & a->chunks[i]) == kBitMask;
    a->chunks[i] &= kBitMask - 1;
    ++i;
  }
  a->bits = bits;
  trim_bits_by_one(a, kBigintChunkBits);
  update_size(a, kBigintChunkBits);
}

Bigint* split(const Bigint* n) {
  Bigint* res = bigint_new(n->size << 1);
  if (n->size == 0) return res;
  res->sign = n->sign;
  res->bits = n->bits;
  res->size = res->capacity;

  chunk* p = res->chunks;
  chunk cur = n->chunks[0];
  int need = kHalfOffset,
      remainder = kBigintChunkBits,
      i = 0;
  while (i < n->size) {
    if (remainder >= need) {
      *p++ |= (cur & ((chunk)1 << need) - 1) << kHalfOffset - need;
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
  if (res->chunks[res->size - 1] == 0) --res->size;
  return res;
}

void combine(Bigint* n) {
  if (n->size == 0) return;
  int need = kBigintChunkBits,
      remainder = kHalfOffset;
  chunk* l = n->chunks;
  chunk* r = n->chunks;
  chunk* end = n->chunks + n->size;
  chunk cur = *r;
  *r = 0;
  while (r < end) {
    if (remainder >= need) {
      *l++ |= (cur & ((chunk)1 << need) - 1) << kBigintChunkBits - need;
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
  Bigint* res = bigint_new(len);
  res->sign = a->sign * b->sign;
  chunk* buf = (chunk*)bigint_malloc(len * sizeof(chunk));
  chunk tmp, carry_mul = 0, carry_add = 0;
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
  if (res->capacity && res->chunks[res->capacity-1] == 0) res->size = res->capacity-1;
  else res->size = res->capacity;
  res->bits = a->bits + b->bits;
  trim_bits_by_one(res, kHalfOffset);
  combine(res);
  return res;
}

#ifdef __cplusplus
}
#endif //__cplusplus
