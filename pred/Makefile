ALL: comp

comp: ../main.c pred.c
	gcc -I../ -O2 -march=native -o comp ../main.c pred.c

test:
	./comp -o enwik8.p ../enwik8
	./comp -d -o enwik8.txt enwik8.p
	md5sum ../enwik8 enwik8.txt
