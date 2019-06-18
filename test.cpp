#include "bigint.c"
#include "gtest/gtest.h"

namespace {

TEST(atobi, HandleZeros) {
  Bigint* n;
  const char* cases[11] = {
    "0",
    "--0",
    "+0",
    "---0",
    "00",
    "--00",
    "++00",
    "-+-00",  
    "0000000000000000000", // 2^63-1 = 9223372036854775807, 19 digits
    "00000000000000000000",
    "00000000000000000000000000000000000"
  };
  for (const char* s : cases) {
    n = atobi(s);
    EXPECT_EQ(n->size, 0);
    EXPECT_GE(n->capacity, n->size);
    EXPECT_EQ(n->chunks[0], 0);
    EXPECT_EQ(n->bits, 0);
    EXPECT_EQ(n->sign, 1);
    delete n->chunks;
    delete n;
  }
}

TEST(atobi, Boundary) {
  Bigint* n;
  
  n = atobi("1");
  EXPECT_EQ(n->size, 1);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 1);
  EXPECT_EQ(n->bits, 1);
  EXPECT_EQ(n->sign, 1);
  delete n->chunks;
  delete n;

  n = atobi("3");
  EXPECT_EQ(n->size, 1);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 3);
  EXPECT_EQ(n->bits, 2);
  EXPECT_EQ(n->sign, 1);
  delete n;

  unsigned long long num = (~0ULL) >> 1; // num = 0b0111...111
  int cnt = snprintf(NULL, 0, "%llu", num);
  // 2^k-1 never be the format of 999...999, 
  // so reserving cnt+1 bytes is enough
  char* s = new char[cnt+1]; 

  snprintf(s, cnt+1, "%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 1);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], num);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8-1);
  EXPECT_EQ(n->sign, 1);
  delete n->chunks;
  delete n;

  ++num; // num = 0b1000...000
  snprintf(s, cnt+1, "%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, 1);
  delete n->chunks;
  delete n;

  delete s;
}

TEST(atobi, UnaryPrefices) {
  Bigint* n;
  
  unsigned long long num = ((~0ULL) >> 1) + 1; // num = 0b1000...000
  int cnt = snprintf(NULL, 0, "%llu", num);
  cnt += 10;
  char* s = new char[cnt]; 

  snprintf(s, cnt, "+%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, 1);
  bigint_destroy(n);

  snprintf(s, cnt, "+%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, 1);
  bigint_destroy(n);

  snprintf(s, cnt, "-%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, -1);
  bigint_destroy(n);

  snprintf(s, cnt, "++%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, 1);
  bigint_destroy(n);

  snprintf(s, cnt, "--%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, 1);
  bigint_destroy(n);

  snprintf(s, cnt, "+-+%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, -1);
  bigint_destroy(n);

  snprintf(s, cnt, "-+-%llu", num);
  n = atobi(s);
  EXPECT_EQ(n->size, 2);
  EXPECT_GE(n->capacity, n->size);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  EXPECT_EQ(n->sign, 1);
  bigint_destroy(n);

  delete s;
}

TEST(bitoa, ReversedAtobi) {
  const char* cases[15] = {
    "0",
    "1",
    "3",
    "65535",
    "65536",
    "2147483647",
    "2147483648",
    "140737488355327",
    "140737488355328",
    "9223372036854775807",
    "-9223372036854775807",
    "9223372036854775808",
    "-9223372036854775808",
    "194892184921349854213168441698561237986513354652189763634584888122150",
    "-194892184921349854213168441698561237986513354652189763634584888122150"
  };
  Bigint* n;
  char* res;
  for (const char* s : cases) {
    n = atobi(s);
    res = bitoa(n);
    EXPECT_STREQ(res, s);
    delete res;
    delete n->chunks;
    delete n;
  }
}

TEST(add_eq, AddWithZero) {
  Bigint* a;
  Bigint* b;
  char* res;

  a = atobi("0");
  b = atobi("0");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "0");
  delete res, a, b;

  a = atobi("0");
  b = atobi("1");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "1");
  delete res, a, b;

  a = atobi("1");
  b = atobi("0");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "1");
  delete res, a, b;
}

TEST(add_eq, Positive) {
  Bigint* a;
  Bigint* b;
  char* res;

  a = atobi("12345");
  b = atobi("12345");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "24690");
  delete res, a, b;

  a = atobi("2147483647");
  b = atobi("1");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "2147483648");
  delete res, a, b;

  a = atobi("1");
  b = atobi("2147483647");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "2147483648");
  delete res, a, b;

  a = atobi("9223372036854775807");
  b = atobi("1");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "9223372036854775808");
  delete res, a, b;

  a = atobi("4895128432161897895613214984561232112688456198122111351");
  b = atobi("9223372036854775808");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "4895128432161897895613214984561232121911828234976887159");
  delete res, a, b;

  a = atobi("9223372036854775808");
  b = atobi("4895128432161897895613214984561232112688456198122111351");
  bigint_add_assign(a, b);
  res = bitoa(a);
  EXPECT_STREQ(res, "4895128432161897895613214984561232121911828234976887159");
  delete res, a, b;
}

TEST(split, Zero) {
  Bigint* a = atobi("0");
  Bigint* res = split(a);
  EXPECT_EQ(res->size, 0);
  EXPECT_GE(res->capacity, res->size);
  EXPECT_EQ(res->chunks[0], 0);
  bigint_destroy(a);
  bigint_destroy(res);
}

TEST(split, One) {
  Bigint* a = atobi("1");
  Bigint* res = split(a);
  EXPECT_EQ(res->size, 1);
  EXPECT_GE(res->capacity, res->size);
  EXPECT_EQ(res->chunks[0], 1);
  bigint_destroy(a);
  bigint_destroy(res);
}

TEST(split, Big) {
  Bigint* a = atobi("171086437689357109634083209420451835789012398474646140128370398");
  // 171086437689357109634083209420451835789012398474646140128370398 = 
  // 0x 00006a77 a45a0780 44ce2b07 6750ba396 66f0526 ca50876b aa549ede
  bigint_chunk ans[7] = {
    0x00006a77, 0xa45a0780, 0x44ce2b07, 0x6750ba39,
    0x666f0526, 0xca50876b, 0xaa549ede
  };

  Bigint* res = split(a);
  EXPECT_EQ(res->size, 7);
  for (int i = 0; i < 7; ++i) {
    EXPECT_EQ(res->chunks[i], ans[7-1-i]);
  }
  bigint_destroy(a);
  bigint_destroy(res);
}

TEST(commbine, Zero) {
  Bigint* a = atobi("0");
  Bigint* a_split = split(a);
  combine(a_split);
  EXPECT_EQ(a_split->size, 0);
  EXPECT_GE(a_split->capacity, a_split->size);
  EXPECT_EQ(a_split->bits, 0);
  EXPECT_EQ(a_split->sign, 1);
  EXPECT_EQ(a_split->chunks[0], 0);
  bigint_destroy(a);
  bigint_destroy(a_split);
}

TEST(commbine, MinusOne) {
  Bigint* a = atobi("-1");
  Bigint* a_split = split(a);
  combine(a_split);
  EXPECT_EQ(a_split->size, 1);
  EXPECT_GE(a_split->capacity, a_split->size);
  EXPECT_EQ(a_split->bits, 1);
  EXPECT_EQ(a_split->sign, -1);
  EXPECT_EQ(a_split->chunks[0], 1);
  bigint_destroy(a);
  bigint_destroy(a_split); 
}

TEST(commbine, Big) {
  Bigint* a = atobi("171086437689357109634083209420451835789012398474646140128370398");
  Bigint* a_split = split(a);
  combine(a_split);
  EXPECT_EQ(a_split->size, a->size);
  EXPECT_GE(a_split->capacity, a->capacity);
  EXPECT_EQ(a_split->bits, a->bits);
  EXPECT_EQ(a_split->sign, a->sign);
  for (int i = 0; i < a->capacity; ++i) {
    EXPECT_EQ(a_split->chunks[i], a->chunks[i]);
  }
  bigint_destroy(a);
  bigint_destroy(a_split);
}

TEST(bigint_mul_bigint_school, Zero) {
  Bigint* a;
  Bigint* b;
  Bigint* res;
  char* s;

  a = atobi("0");
  b = atobi("0");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "0");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("0");
  b = atobi("698279837967019823756324324");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "0");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("698279837967019823756324324");
  b = atobi("0");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "0");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;
}

TEST(bigint_mul_bigint_school, One) {
  Bigint* a;
  Bigint* b;
  Bigint* res;
  char* s;

  a = atobi("1");
  b = atobi("1");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "1");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("1");
  b = atobi("698279837967019823756324324");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "698279837967019823756324324");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("698279837967019823756324324");
  b = atobi("1");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "698279837967019823756324324");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("-1");
  b = atobi("698279837967019823756324324");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "-698279837967019823756324324");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("698279837967019823756324324");
  b = atobi("-1");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "-698279837967019823756324324");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;

  a = atobi("-1");
  b = atobi("-698279837967019823756324324");
  res = bigint_mul_bigint_school(a, b);
  s = bitoa(res);
  EXPECT_STREQ(s, "698279837967019823756324324");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;
}

TEST(bigint_mul_bigint_school, Bounary) {
  char buf[100];
  bigint_chunk na = ((bigint_chunk)1 << (sizeof(bigint_chunk) << 1)) + 1;
  bigint_chunk nb = (na >> 1) + 1;
  
  _i64toa(na, buf, 10);
  Bigint* a = atobi(buf);
  _i64toa(nb, buf, 10);
  Bigint* b = atobi(buf);
  Bigint* res = bigint_mul_bigint_school(a, b);
  char* s = bitoa(res);
  _i64toa(na * nb, buf, 10);
  EXPECT_STREQ(s, buf);
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;
}

TEST(bigint_mul_bigint_school, Big) {
  Bigint* a = atobi("17389123634806831236074612378046861121035");
  Bigint* b = atobi("9512396519651236523165123066512035456230651216");
  Bigint* res = bigint_mul_bigint_school(a, b);
  char* s = bitoa(res);
  EXPECT_STREQ(s, "165412239143521561104276677978572512625506496660737407710010357628512987639509845928560");
  bigint_destroy(a);
  bigint_destroy(b);
  bigint_destroy(res);
  delete s;
}

}
