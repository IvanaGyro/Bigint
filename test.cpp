#include <iostream>
#include "bigint.h"
#include <string.h>


using namespace std;

int main(int argc, char **argv)
{
  bigint_chunk* num;
  int len;
  
  Bigint* n;
  
  n = atobi("0");
  printf("n.chunks: %p\nn.len: %p\nn.bits: %p\n", n->chunks, n->len, n->bits);
  printf("%X\n", n->chunks[n->len-1]);
  cout << bitoa(n->chunks, n->len) << endl;
  
  n = atobi("0571967239523523759103496341235627");
  printf("n.chunks:%p\nn.len: %p\nn.bits: %p\n",n->chunks, n->len, n->bits);
  printf("%llX\n", n->chunks[n->len-1]);
  cout << bitoa(n->chunks, n->len) << endl;
}
