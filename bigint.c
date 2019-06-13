#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "string.h"
#include "stdlib.h"
#include "bigint.h"

#include "stdio.h"

Bigint bigint_default = {NULL, 0, 0};

int atobi(const char* in, bigint_chunk** out) {
  int len = strlen(in);
  char* buf = (char*)malloc(len + 1);
  strcpy(buf, in);
  char*s = buf;
  while (*s == '0') {
    ++s;
    --len;
  }
  if (len == 0) {
    *out = (bigint_chunk*)malloc(kBigintChunkSize);
    (*out)[0] = 0;
    return 1;
  }
  
  int chunk_len = (int)(len * kBigintLog2_10 / kBigintChunkBits + 1);
  bigint_chunk* num = (bigint_chunk*)malloc(chunk_len * kBigintChunkSize);
  memset(num, 0, chunk_len * kBigintChunkSize);

  int tail = len - 1;
  bigint_chunk* cur_chunk = num;
  bigint_chunk cur_mask = 1;
  char carry, tmp;
  int i, l, r;

  while (*s) {
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
  *out = num;
  return chunk_len;
}

char* bitoa(const bigint_chunk* num, int len) {
  char* s = (char*)malloc(len * kBigintChunkBits * kBigintLog2 + 2);
  memset(s, 0, len * kBigintChunkBits * kBigintLog2 + 2);
  char* buf = (char*)malloc(kBigintChunkBits * kBigintLog2 + 2);
  *s = '0';
  int i;
  char* l;
  char* r;
  char tmp;
  bigint_chunk mask, upper_bound = kBigintMask >> 1;
  bigint_chunk multiplier = 1;
  for (i=len-1; i >= 0; --i) {
    mask = upper_bound;
    while (mask) {
      multiplier <<= 1;
      if (num[i] & mask || multiplier == upper_bound) {
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
        if (num[i] & mask) add(s, "1");
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
  return s;
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

  char* buf = (char*)malloc(al + bl + 1);
  char* a_copy = (char*)malloc(al + 1);
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

#ifdef __cplusplus
}
#endif //__cplusplus
