#include <stdlib.h> // calloc
#include <stdint.h> // uint8_t
#include <stdio.h>  // FILE

#define HASH_SIZE (1<<20)
#define HASH_MASK (HASH_SIZE-1)
#define HASH(state, c) ((state << 4) ^ c) & HASH_MASK

int compress(FILE *src, FILE *dest) {
  int c;
  uint32_t size = 0;
  uint32_t state = 0;
  uint8_t flags = 0;
  int loop = 0;
  int count = 0;
  uint8_t literals[8];

  uint8_t *hash = calloc(HASH_SIZE, 1);

  // Write file size at start!
  fwrite(&size, sizeof(size), 1, dest);

  while( (c = fgetc(src)) != EOF ) {
    ++size;

    flags <<= 1;

    if(c != hash[state]) {
      flags |= 1;
      literals[count++] = c;
    }
    hash[state] = c;
    state = HASH(state, c);

    if(++loop == 8) {
      fputc(flags, dest);
      fwrite(literals, 1, count, dest);

      loop = 0;
      count = 0;
      flags = 0;
    }
  }
  if(loop) {
    fputc(flags, dest);
    fwrite(literals, 1, count, dest);
  }

  fseek(dest, 0, SEEK_SET);
  fwrite(&size, sizeof(size), 1, dest);
}


int decompress(FILE *src, FILE *dest) {
  uint8_t c;
  uint32_t state = 0;
  uint32_t size;
  int flags;
  int count;

  uint8_t *hash = calloc(HASH_SIZE, 1);

  fread(&size, sizeof(size), 1, src);

  while(size) {
    flags = fgetc(src);
    for(count=0; count < 8; count++) {
      if(flags & 0x80) {
        c = fgetc(src);
      } else {
        c = hash[state];
      }
      fputc(c, dest);
      --size;

      hash[state] = c;
      state = HASH(state, c);
      flags <<= 1;
    }
  }
}
