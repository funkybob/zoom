ALL: comp

comp: ../main.c basiclz.c
	gcc -Wall -Wextra -Werror -pedantic -I../ -Ofast -march=native -mcrc32 -o comp ../main.c basiclz.c

test:
	./comp -o enwik8.p ../enwik8
	./comp -d -o enwik8.txt enwik8.p
	md5sum ../enwik8 enwik8.txt
	ls -l enwik8.p
