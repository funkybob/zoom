ALL: comp

comp: main.c
	gcc -O2 -o comp main.c

test:
	./comp -o enwik8.p ../enwik8
	./comp -d -o enwik8.txt enwik8.p
