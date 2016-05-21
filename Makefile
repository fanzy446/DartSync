all: test/downloader

peer/p2p.o: common/constants.h common/filetable.h peer/p2p.h
	gcc -Wall -pedantic -std=c99 -g -c peer/p2p.c -o peer/p2p.o

peer/p2p: peer/p2p.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -c peer/p2p.c peer/p2p.o -o peer/p2p

test/downloader: test/downloader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -c test/downloader.c peer/p2p.o -o test/downloader

clean:
	rm -rf peer/*.o
	rm -rf test/*.o



