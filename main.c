#include <stdlib.h> // malloc
#include <stdio.h>  // FILE
#include <unistd.h> // getopt

#include <errno.h>

#include "comp.h"

int main(int argc, char **argv) {
  int opt;
  int mode = 'c';  // Compression
  FILE *fin, *fout = stdout;

  while( (opt = getopt(argc, argv, "do")) != -1) {
    switch(opt) {
    case 'd':
      mode = 'd';
      break;
    case 'o':
      fout = fopen(argv[optind++], "w");
      break;
    }
  }

  if (optind < argc) {
    fin = fopen(argv[optind++], "r");
  } else {
    fin = stdin;
  }


  struct buffer *src = malloc(sizeof(struct buffer));
  src->data = malloc(1 << 24);
  src->size = 0;

  struct buffer * dest = malloc(sizeof(struct buffer));
  dest->data = malloc(1 << 24);
  dest->size = 0;


  if (mode == 'c') {

    while(1) {
      src->size = fread(src->data, 1, 1 << 24, fin);
      if (src->size == 0) break;

      // original size
      fwrite(&src->size, 4, 1, fout);

      size_t dsize = compress(src, dest);

      // compressed size
      fwrite(&dsize, 4, 1, fout);
      fwrite(dest->data, 1, dsize, fout);
    }

  } else {

    while(1) {
      size_t size = 0;
      if(fread(&size, 4, 1, fin) == 0) break;

      fread(&src->size, 4, 1, fin);
      fread(src->data, 1, src->size, fin);

      size_t dsize = decompress(src, dest, size);

      fwrite(dest->data, 1, dsize, fout);
    }
  }

  free(src->data);
  free(src);
  free(dest->data);
  free(dest);
}
