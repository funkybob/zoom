ALL: comp

comp: Makefile main.c basiclz.c basiclz.h
	gcc -Wall -Wextra -Werror -pedantic -Ofast -march=native -o comp main.c basiclz.c

clean:
	rm -f comp enwik* *.log *.p

test: comp
	./comp -l -o enwik8.p ../enwik8 > tc.log
	./comp -d -o enwik8.txt enwik8.p > td.log
	md5sum ../enwik8 enwik8.txt
	ls -l enwik8.p
