#include "bigint.h"
#include "gtest/gtest.h"

namespace {

TEST(atobi, HandleZeros) {
  Bigint* n;
  const char* cases[5] = {
    "0",
    "00",
    "0000000000000000000", // 2^63-1 = 9223372036854775807, 19 digits
    "00000000000000000000",
    "00000000000000000000000000000000000"
  };
  for (const char* s : cases) {
    n = atobi(s);
    EXPECT_EQ(n->chunks[0], 0);
    EXPECT_EQ(n->len, 1);
    EXPECT_EQ(n->bits, 0);
    delete n->chunks;
    delete n;
  }
}

TEST(atobi, Boundary) {
  Bigint* n;
  
  n = atobi("1");
  EXPECT_EQ(n->chunks[0], 1);
  EXPECT_EQ(n->len, 1);
  EXPECT_EQ(n->bits, 1);
  delete n->chunks;
  delete n;

  n = atobi("3");
  EXPECT_EQ(n->chunks[0], 3);
  EXPECT_EQ(n->len, 1);
  EXPECT_EQ(n->bits, 2);
  delete n->chunks;
  delete n;

  unsigned long long num = (~0ULL) >> 1; // num = 0b0111...111
  int cnt = snprintf(NULL, 0, "%llu", num);
  // 2^k-1 never be the format of 999...999, 
  // so reserving cnt+1 bytes is enough
  char* s = new char[cnt+1]; 

  snprintf(s, cnt+1, "%llu", num);
  n = atobi(s);
  EXPECT_GE(n->len, 1);
  EXPECT_EQ(n->chunks[0], num);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8-1);
  delete n->chunks;
  delete n;

  ++num; // num = 0b1000...000

  snprintf(s, cnt+1, "%llu", num);
  n = atobi(s);
  EXPECT_GE(n->len, 2);
  EXPECT_EQ(n->chunks[0], 0);
  EXPECT_EQ(n->chunks[1], 1);
  EXPECT_EQ(n->bits, sizeof(unsigned long long)*8);
  delete n->chunks;
  delete n;

  delete s;
}

TEST(bitoa, ReversedAtobi) {
  const char* cases[12] = {
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
    "9223372036854775808",
    "194892184921349854213168441698561237986513354652189763634584888122150"
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

}
