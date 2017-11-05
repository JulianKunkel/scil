// Test of Huffman code implementation
// Used as prefix for SCIL allquant algorithm
// Author: Oliver Pola <5pola@informatik.uni-hamburg.de>

#include <algo/huffman.h>
// CMake hat problems linking this
#include <algo/huffman.c>
#include <stdio.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte) \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define TESTSIZE 7

int main() {
  // Example taken from "Cormen, Leiserson - Introduction to Algorithms"
  // Chapter 16.3
  // Added another g with count 0, that we want to support
  // As such it shouldn't change other results
  huffman_entity test[TESTSIZE];
  test[0].data = "a";
  test[0].count = 45;
  test[1].data = "b";
  test[1].count = 13;
  test[2].data = "c";
  test[2].count = 12;
  test[3].data = "d";
  test[3].count = 16;
  test[4].data = "e";
  test[4].count = 9;
  test[5].data = "f";
  test[5].count = 5;
  test[6].data = "g";
  test[6].count = 0;

  huffman_encode(test, TESTSIZE);

  for(int i = 0; i < TESTSIZE; i++) {
    printf("%s %2lu -> mask = "BYTE_TO_BINARY_PATTERN", value = "
      BYTE_TO_BINARY_PATTERN", bits = %d\n",
      (char *)test[i].data, test[i].count,
      BYTE_TO_BINARY(test[i].bitmask), BYTE_TO_BINARY(test[i].bitvalue),
      test[i].bitcount);
  }

  int error =
    (test[0].bitmask != 128) ||
    (test[0].bitvalue != 0) ||
    (test[0].bitcount != 1) ||
    (test[1].bitmask != 224) ||
    (test[1].bitvalue != 160) ||
    (test[1].bitcount != 3) ||
    (test[2].bitmask != 224) ||
    (test[2].bitvalue != 128) ||
    (test[2].bitcount != 3) ||
    (test[3].bitmask != 224) ||
    (test[3].bitvalue != 224) ||
    (test[3].bitcount != 3) ||
    (test[4].bitmask != 240) ||
    (test[4].bitvalue != 208) ||
    (test[4].bitcount != 4) ||
    (test[5].bitmask != 240) ||
    (test[5].bitvalue != 192) ||
    (test[5].bitcount != 4) ||
    (test[6].bitmask != 0) ||
    (test[6].bitvalue != 1) ||
    (test[6].bitcount != 0);

  if(error) {
    printf("Unexpected results!\n");
    return 1;
  } else
    return 0;
}
