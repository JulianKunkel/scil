// Huffman code implementation
// Used as prefix for SCIL allquant algorithm
// Author: Oliver Pola <5pola@informatik.uni-hamburg.de>

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdlib.h>

typedef struct huffman_entity {
  void* data;
  size_t count;
  unsigned char bitmask;
  unsigned char bitvalue;
  unsigned char bitcount;
} huffman_entity;

// pre: data, count is set (count = 0 is allowed)
// post: bitmask, bitvalue, bitcount will be set
void huffman_encode(huffman_entity* entities, size_t size);

#endif // HUFFMAN_H
