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

  if (mode == 'c') {
    struct buffer src;

    src.size = 1 << 24;
    src.data = malloc(1 << 24);

    while(1) {
      src.size = fread(src.data, 1, 1 << 24, fin);
      if (src.size == 0) break;

      // original size
      fwrite(&src.size, 4, 1, fout);

      struct buffer *dest = compress(&src);

      // compressed size
      fwrite(&dest->size, 4, 1, fout);
      fwrite(dest->data, 1, dest->size, fout);

      free(dest->data);
      free(dest);
    }

  } else {

    struct buffer *src = malloc(sizeof(struct buffer));
    src->data = malloc(1 << 24);
    src->size = 0;

    while(1) {
      size_t size = 0;
      if(fread(&size, 4, 1, fin) == 0) break;

      fread(&src->size, 4, 1, fin);
      fread(src->data, 1, src->size, fin);

      struct buffer *dest = decompress(src, size);

      fwrite(dest->data, 1, dest->size, fout);
      free(dest->data);
      free(dest);

    }
    free(src->data);
    free(src);
  }
}
