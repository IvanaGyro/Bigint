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

}
