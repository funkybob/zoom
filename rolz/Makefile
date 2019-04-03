ALL: comp

comp: ../main.c rolz.c
	gcc -g -O2 -o comp ../main.c rolz.c

test:
	./comp -o enwik8.p ../enwik8 > tc.log
	./comp -d -o enwik8.txt enwik8.p > td.log
	md5sum ../enwik8 enwik8.txt
