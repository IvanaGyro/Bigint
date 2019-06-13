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

}
