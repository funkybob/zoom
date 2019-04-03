#include <stdio.h>  // FILE
#include <unistd.h>  // getopt

extern void compress(FILE *, FILE *);
extern void decompress(FILE *, FILE *);

int main(int argc, char **argv) {
  int opt;
  int mode = 'c';  // Compression
  FILE *src, *dest = stdout;

  while( (opt = getopt(argc, argv, "do")) != -1) {
    switch(opt) {
    case 'd':
      mode = 'd';
      break;
    case 'o':
      dest = fopen(argv[optind++], "w");
      break;
    }
  }

  if (optind < argc) {
    src = fopen(argv[optind++], "r");
  } else {
    src = stdin;
  }

  if (mode == 'c') {
    compress(src, dest);
  } else {
    decompress(src, dest);
  }
}
