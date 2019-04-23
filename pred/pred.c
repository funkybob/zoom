#include <stdlib.h> // calloc
#include <stdint.h> // uint8_t
#include <stdio.h>  // FILE
#include <string.h> // malloc

#include "comp.h"

#define HASH_SIZE (1<<20)
#define HASH_MASK (HASH_SIZE-1)
#define HASH(state, c) ((state << 4) ^ c) & HASH_MASK


size_t compress(struct buffer* src, struct buffer* dest) {
  uint32_t state = 0;
  uint8_t flags = 0;

  int loop = 0; // flush flags + literals every 8 iterations
  int lit_counter = 0;
  uint8_t lit_pending[8];

  uint8_t *hash = calloc(HASH_SIZE, 1);

  size_t sptr = 0;

  dest->size = 0;

  while(sptr < src->size) {

    uint8_t c = src->data[sptr++];

    flags <<= 1;

    if(c != hash[state]) {
      flags |= 1;
      lit_pending[lit_counter++] = c;
    }
    hash[state] = c;
    state = HASH(state, c);

    if(++loop == 8) {
      dest->data[dest->size++] = flags;
      memcpy(&dest->data[dest->size], lit_pending, lit_counter);
      dest->size += lit_counter;

      loop = 0;
      lit_counter = 0;
      flags = 0;
    }
  }
  if(loop) {
    dest->data[dest->size++] = flags;
    memcpy(&dest->data[dest->size], lit_pending, lit_counter);
    dest->size += lit_counter;
  }

  return dest->size;
}


size_t decompress(struct buffer *src, struct buffer *dest, size_t output_size) {
  uint8_t c;
  uint32_t state = 0;
  size_t sptr = 0;

  int flags;
  int count;

  dest->size = 0;

  uint8_t *hash = calloc(HASH_SIZE, 1);

  while(dest->size < output_size) {
    flags = src->data[sptr++];

    for(count=0; count < 8; count++) {
      if(flags & 0x80) {
        c = src->data[sptr++];
      } else {
        c = hash[state];
      }
      dest->data[dest->size++] = c;

      hash[state] = c;
      state = HASH(state, c);
      flags <<= 1;
    }
  }

  return dest->size;
}
